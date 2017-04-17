#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>



void math( char *data)
{
	printf("math\n");	
	assert(data);
	int i = 0;
	int j =0;
	char* val[3];
	char* start= data;
	while(*start)
	{
		if(*start=='=')
		{
			val[j]=start+1;
			j++;
		}	
		if(*start=='&')
		{
			*start=0;
		}
		start++;

	}
	val[j]=NULL;
	int left = atoi(val[0]);
	int right = atoi(val[1]);

	printf("<html>");
	printf("<h1><br/>");
	printf("<h1>%d + %d = %d</h1><br>",left,right,left+right);
	printf("</h1>");
	printf("</html>");

}
int main() //先往管道里面写，等子进程正常退出的时候在管道里面读，否则不读
{
	//printf("%s   %s    %s \n",getenv("METHOD"),getenv("CONTENT_LENGTH"),getenv("QUERY_STRING"));
	//printf("cgi is running\n");
	int ret = 0;
	char cgi_data[10240];
	int content_len;
	char* query_string;
	if(!getenv("METHOD"))
	{
		//echo_errno();
		//printf_log();
		ret =  -1;
		return ret;
	}
	else
	{
		printf("%s\n",getenv("METHOD"));
		char* method = getenv("METHOD");
		if(strcasecmp("GET",method)==0 )
		{
			if(!getenv("QUERY_STRING"))
			{
				return -2;
			}
			query_string = getenv("QUERY_STRING");
			strcpy(cgi_data,query_string);
			printf("cgi_data %s\n",cgi_data);
		}
		else
		{
			int i = 0;
			char ch = '\0';
			if(!getenv("CONTENT_LENGTH"))
			{
				ret = -3;
				return ret;
			}
			content_len=atoi(getenv("CONTENT_LENGTH"));
			printf("content_len : %d \n",content_len);
			while(i < content_len)
			{
				read(0,&ch,1);
				cgi_data[i]= ch;
				i++;
			}
			cgi_data[i] = '\0';	
		}
		printf("cgi_data: %s\n",cgi_data);
		math(cgi_data);	
	}
//
//		
//	
	
	return 0; 
}
