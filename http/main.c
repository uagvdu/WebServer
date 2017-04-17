#include "httpd.h"

int main(int argc ,char* argv[])
{
	if(argc!=3)
	{
		printf("%s [local_ip] [local_port]\n",argv[0]);
		exit(1);
	}
	
	int listen_sock=startup(argv[1],atoi(argv[2]));
	//daemon(0,0);//守护进程与终端无关，可以使用#ifdef debuf模式写到前台，如果是以release 模式。则写到后台：错误消息重制到日志：

	while(1)
	{
		struct sockaddr_in cli_sock;
		socklen_t len = sizeof(cli_sock);
		int link_sock = accept(listen_sock,(struct sockaddr*)&cli_sock,&len);
		if(link_sock < 0)
		{
			//打印日志，并且响应客户端，请求服务失败
		//	printf_log("client link failed!\n",WORNNING);
		//	link_fail();
			continue;
		}
		pthread_t tid;
		int errcode=pthread_create(&tid,NULL,accept_sock,(void*)link_sock);
		if(errcode > 0)
		{
		//	printf_log("pthread create failed!\n",FATAL);
			exit(2);
		}
		
	}

	return 0;
}
