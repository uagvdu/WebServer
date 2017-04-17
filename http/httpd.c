#include"httpd.h"

int startup(char* _ip,int _port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
	//	printf_log("socket create failed!\n",FATAL);
		exit(3);
	}
	int opt =1;
	int stat=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	struct sockaddr_in server_sock;
	server_sock.sin_family = AF_INET;
	server_sock.sin_port= htons(_port);
	server_sock.sin_addr.s_addr=inet_addr(_ip);
	socklen_t len = sizeof(server_sock);

	if(bind(sock,(struct sockaddr*)&server_sock,len)<0)
	{
	//	printf_log("bind failed!\n",FATAL);
		exit(4);	
	}
	if(listen(sock,5)<0)
	{
	//	printf_log("listen failed!\n",FATAL);
		exit(5);
	}
	return sock;
}

void* accept_sock(void *fd)
{
	int sock = (int)fd;
	pthread_detach(pthread_self());
	handler_sock(sock);
}

//int get_line(int sock,char buf[])
//{
//	char c = '\0';
//	int i = 0; 
//	//换行符有三种情况：\r \r\n \n
//	//
//	
//	if(recv(sock,&c,1,0)<=0)
//	{
//		//printf_log("recv failed !\n",WORNNING);
//		return -1; 
//	}
//	while((i< SIZE-1) &&(c!='\r')&&(c!='\n'))
//	{
//		buf[i]=c;
//		if(recv(sock,&c,1,0)<=0)
//		{
//			//printf_log("recv failed !\n",WORNNING);
//			return -1; 
//		}
//		i++;	
//	}
//	if(c=='\r'|| c=='\n')
//	{
//		if(c == '\n')
//		{
//			buf[i]='\n';
//			i++;
//		}
//		else if(c=='\r'&&recv(sock,&c,1,MSG_PEEK)=='\n')	
//		{
//			buf[i]='\n';
//			i++;
//		}
//		else 
//		{
//			c = '\n';
//			buf[i]='\n';
//			i++;
//		}
//	}
//		
//
//	buf[i]='\0';
//	printf("buf:%s\n",buf);
//	//要不要设置buf的最后一位为\0?:buf读取的是请求行，加上作为限制
//	return i;
//}
static int get_line(int sock,char* buf,int sz)
{
	assert(buf);
	char c = '\0';
	int i =0;

	while(i<sz-1&&c!='\n')
	{	
		ssize_t s = recv(sock,&c,1,0);
		if(s > 0&&c== '\r')
		{	
			if(recv(sock,&c,1,MSG_PEEK)>0)
			{
				if(c == '\n')
				{
					recv(sock,&c,1,0);
				}
				else
				{
					c = '\n';
				}
				
			}
		}

		buf[i++]=c;
	}
	buf[i]='\0';
	return i;
}




static void clear_head(int sock)//也要先把请求处理完，再往里面响应
{
	//printf("clear_head\n");
	int ret;
	char buf[SIZE];
	do
	{
		ret=get_line(sock,buf,SIZE);
		printf("%s",buf);					//recv在handler_sock的时候已经读取一次了，所以在while循环中不会再从头开始读取
	}while(ret!=1&&strcmp(buf,"\n")!=0);
}

static int excu_cgi(const char *method,int sock,const char* path,const char *query_string,ssize_t sz)
{
	//GET or POST or bin：不用考虑可执行文件的问题，因为到了这步，肯定是可执行文件 
	
	// cgi :fork/exec -> env ->
	//printf("excu_cgi\n");	

	printf("%s \n",path);
	int content_len = -1;
	if(strcasecmp(method,"GET")==0)
	{
		printf("Entering cgi\n");
		clear_head(sock);
	}
	else 
	{
		int ret = 0;
		char buf[SIZE];
		do
		{
			ret = get_line(sock,buf,SIZE);
			printf("%s",buf);
			if(ret>0&&strncmp(buf,"Content-Length: ",16)==0)
			{
				content_len=atoi(buf+16);
			}
		}while(ret!=1&&strcmp(buf,"\n")!=0);
		printf("content_len: %d\n",content_len);
		if(content_len < 0)
		{
			//errno();
			//printf_log();
			ret = 11;
			return ret;
		}

	//到这里完成了get和post的参数获取方式
		
	}
	//printf("status_line\n");
		char* status_line = "Http/1.0 200 OK\r\n";
		send(sock,status_line,strlen(status_line),0);
	//	send(sock,"helloWorld!",strlen("helloWorld!"),0);
		const char *content_type="Content-Type:text/html;charset=ISO-8859-1\r\n";
		send(sock,content_type,strlen(content_type),0);
		send(sock,"\r\n",strlen("\r\n"),0);
		
	/*
	 * 如果以管道传递path等变量的时候还需要对cgi获得的参数进行划分
	 * 所以以传值的方式通过环境变量进行传参：子进程继承父进程的环境变量
	 * 子进程会继承fork之前所有的文件描述符：
	 * */
		int ret = 0;
		char method_env[10240];
		char query_string_env[10240];
		char content_len_env[10240];
		int input[2]={0};;
		int output[2]={0};;
		int in= pipe(input);
		int out= pipe(output);
		if(in != 0||out !=0 )
		{
			printf("pipe errno!\n");
			//echo_errno();
			//printf_log();
			ret = 12;
			return ret;
		}
		
	//	printf("Create fork\n");
		pid_t fk=fork();
		if(fk<0)
		{
			//echo_errno();
			//printf_log();
			ret = 12;
			return ret;
		}else if(fk==0)
		{
			close(input[1]);
			close(output[0]);
			close(sock); //防止误写
		
			dup2(input[0],0);
			dup2(output[1],1);
			//执行cgi :need参数->环境变量
			//方法，资源路径，参数：可以完成的是exec、程序替换：执行cgi 
			//目前cgi知道目标的：方法，资源路径，参数，-> 能唯一的确定服务器上的资源，如何确定，需要不知道什么？
			//管道的输入输出是做什么用的？？？
			//为什么要用重定向，重定向能带来什么？以及父进程需要往子进程中写什么？
			//write(output[1],"hellow\n",10);	
		//	pr:intf("child :fork is success \n");	
				
			sprintf(method_env,"METHOD=%s",method);
			putenv(method_env);
	
			if(strcasecmp(method,"GET")== 0)
			{	
				sprintf(query_string_env,"QUERY_STRING=%s",query_string);
				putenv(query_string_env);
			}
			else
			{
				sprintf(content_len_env,"CONTENT_LENGTH=%d",content_len);//每一个环境变量都是字符串：故而不存在其它类型转字符串,
														 //直接用sprintf函数格式化输出到对应的变量中。
				putenv(content_len_env);
				
			}
	
			execl(path,path,NULL);
			printf("execl failed\n");
			exit(1);

		}else
		{	
			close(input[0]);
			close(output[1]);
			char buf[SIZE];
		//	recv(sock,buf,SIZE,0);
//			printf("%s\n",buf);
			char c= '\0';
			if(strcasecmp(method,"POST")==0)
			{
				int i =0;
				for(;i<content_len;i++)
				{
					recv(sock,&c,1,0);
					printf("father: %c\n",c);
					write(input[1],&c,1);
				}

			}

			c='\0';
			while(read(output[0],&c,1)>0)
			{
				printf("%c\n",c);
				send(sock,&c,1,0);
			}

		//	if(read(output[0],&c,1)<0)
		//	{
		//		printf(" <0\n");
		//	}
		//	else if(read(output[0],&c,1)==0)
		//	{
		//		printf("==0\n");
		//	}
		//	else
		//	{
		//		printf(" >0 \n");
		//	}
			waitpid(fk,NULL,0);
			printf("wait child is success!\n");
		}
	close(input[1]);
	close(output[0]);
	return ret;
}
static int echo_www(int sock,char* path,ssize_t sz)//将一个非cgi模式的目标资源直接进行响应给web浏览器？，如何做？:还需要保证读取完整个请求报文，：空行处
{

	//printf("echo_www\n");
	int fd = open(path,O_RDONLY); //path 是一个相对路径，但对于浏览器来说它是绝对路径，只不过根目录是wwwroot
    if(fd < 0)
	{
		//echo_errno();
		//printf_log();
		return 9;
	}	
	char line[SIZE];
	sprintf(line,"HTTP/1.0 200 OK\r\n\r\n");
	send(sock,line,strlen(line),0);
	if(sendfile(sock,fd,NULL,sz)<0)
	{
		//echo_errno();
		return 10;
	}
	//close(fd);
}


int handler_sock(int sock)
{
	int ret = 0; 
	//请求：请求行：读取第一行：确定方法，url，以及http版本
	char buf[SIZE];
	int sz;
	if((sz=get_line(sock,buf,SIZE))<0)
	{
		//echo_errno();
		ret = 6;
		goto end;
	}
	//printf("%s",buf);
	char *query_string = NULL;//url的参数
	char method[10];
	char url[SIZE];
	char http[10];
	int cgi=0 ; 
	char path[10240];
	int i =0;
	int j = 0;
	while((i<sz)&&(j<sizeof(method)-1)&&!isspace(buf[i]))
	{
		method[j]=buf[i];
		i++;
		j++;
	}
	method[i] = 0;
	i++;
	j = 0;
	if((strcasecmp("GET",method))&&(strcasecmp("POST",method)))//strcasecmp :成功则返回值为0，该函数确定该服务器所能使用的方法只能是这两个
	{
		//echo_errno();
		ret=7;
		//printf_log("more method!\n",WORNNING);
		goto end;
	}
	
	while((i<sz)&&(j<sizeof(url)-1)&&!isspace(buf[i]))
	{
		url[j]=buf[i];
		i++;
		j++;
	}
	url[i] = 0;
	i++;
	j=0;
	//url已经确定，接下来根据是否带参决定哪一种方法
	//写的是http1.0版本，所以需要考虑究竟是哪种方法：GET or POST：方法决定了参数的传递方式:根据方法的不同，决定了query_string获取参数的方式不同
	if(!strcasecmp("GET",method))
	{
		query_string=url;
		while(*query_string!='?'&&*query_string!='\0')
		{
			query_string++;
		}
		//query_string为url的参数+?部分,而url保持不变
		if(*query_string=='?')
		{
			*query_string='\0'; //这句话的作用是使得url变为指向资源的路径
			query_string++;
			cgi=1;	//GET有参	
		}

	}
	else{//post
		cgi=1;
	}
	
	while((i<sz)&&(j<sizeof(http)-1)&&!isspace(buf[i]))
	{
		http[j]=buf[i];
		i++;
		j++;
	}
	http[i]=0;
	//Web获取请求行成功：
	
	//请求成功了，还有特殊情况需要考虑：url指向的数据类型
	sprintf(path,"wwwroot%s",url);//格式化输出到path中 ------------------->数组可以用strlen？ok

	if(path[strlen(path)-1] == '/')
	{
		strcat(path,"index.html");
	}

	//请求资源的类型：
	struct stat st;
	if(stat(path,&st)<0)
	{
		//echo_errno();
		ret = 8;
		goto end;
	}
	else
	{//该函数返回关于一个文件的信息，大于0，则表示存在，但无法确定是否为普通的文件资源
		
		if(S_ISDIR(st.st_mode))//stat内部的一个判断st类型的应该是函数吧
		{
			strcat(path,"index.html");
		}
		else if((st.st_mode & S_IXUSR)||(st.st_mode & S_IXGRP)||(st.st_mode & S_IXOTH) )
		{
			cgi =1;/*url的参数应该是为了传递给服务器的可执行文件，即宏观意义上的：对服务器上的进程而言。web浏览器就是
				   其的标准输入和标准输出，若请求的资源本身就是一个可执行文件，cgi就当然设置为1 ： 
				   即使url是get方法，且无参	：cgi的功能就是进行c/s之间的交流*/		
		}

	}
	//问：除了可执行文件可以传参，还有什么文件可以接收参数？貌似没有
	//响应：
		if(cgi==1) //cgi= 1 :post方法，get带参->目标资源为可执行文件
		{
			excu_cgi(method,sock,path,query_string,st.st_size);
			//ret =echo_www(sock,path,st.st_size);
		}
		else //get 无参，目标资源不是可执行文件，直接响应
		{
			clear_head(sock);
			ret =echo_www(sock,path,st.st_size);
		}
	
end:
	close(sock);
	return ret; 
}

