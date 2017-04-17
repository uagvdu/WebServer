#ifndef _HTTPD_H
#define _HTTPD_H
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/sendfile.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<pthread.h>
#include<sys/stat.h>
#include<string.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<assert.h>

#define SIZE 40960
#define NORMAL 1
#define WORNNING 2
#define FATAL 3

int startup(char* _ip,int _port);
void printf_log(char *str,int level);
//void link_fail();
int handler_sock(int sock);
void* accept_sock(void *fd);


#endif
