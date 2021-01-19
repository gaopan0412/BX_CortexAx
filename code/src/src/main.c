#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include <pthread.h>
#include <errno.h>
#include "client.h"
#include <string.h>
#include <time.h>

#define TRUE  1
#define FALSE 0

/*config file path*/
char pconfig[] = "./config.txt";


static int get_systime(void)
{
    struct tm *t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);
    Debugs(LOGLEVEL, "%4d/%02d/%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return 0;
}

/*dstinaion ip connect*/
void* DstLinkHandle(void* arg)
{

	char buff[] = "hello dst station\r\n";
	char buf[100] = {0};
	int dstskfd = 0;

	InitArgs_t* initarg = (InitArgs_t*)arg;
	if ( Tcp_ConnectDst(arg, dstskfd) < 0) {
		Debugs(LOGLEVEL, "dst socket connect failed!\r\n");
		exit(-1);
	}
    Debugs(LOGLEVEL, "dst socket fd:%d\r\n", dstskfd);

	while (1) {
		send(initarg->dstfd, (unsigned char*)buff,  strlen(buff), 0);
		int n = recv(initarg->dstfd, buf, 100, 0);
		if ( n > 0) {
			Debugs(LOGLEVEL, "[dst]%s", buf);	
			get_systime();
		}

		sleep(1);
	}
}

/*base station connet*/
void* BaseStationLinkHandle(void* arg)
{
	int basefd = 0;
	char buff[] = "hello base station\r\n";
	char buf[10] = {0};

	InitArgs_t* initarg = (InitArgs_t*)arg;
	if (Tcp_ConnectBase(arg, basefd) < 0)  {
		Debugs(LOGLEVEL, "socket connect failed!\n");
		exit(-1) ;
	}
	Debugs(LOGLEVEL, "base socket fd:%d\r\n", initarg->basefd);

	while (1) {
		//printf("%s:%s\r\n", initarg->baseip, initarg->baseport);
		send(initarg->basefd, (unsigned char*)buff, strlen(buff), 0);
		int n  = recv(initarg->basefd, buf, 10, 0);
		if (n > 0) {
			Debugs(LOGLEVEL, "[base]%s\n", buf);
			get_systime();
		}

		sleep(1);
	}
}

int main(int argc, char* argv[])
{
	int ret = 0;
	pthread_t dstlink, baselink;
	InitArgs_t initargs;
	FILE* pcfgfile = fopen(pconfig, "r");

	if ( !pcfgfile ) {
		printf("open config file failed!\r\n");
		return -1;
	} else {
		printf("open config file success\r\n");
	}

	get_initargs(pcfgfile, &initargs);


	if (ret = pthread_create(&dstlink, NULL, DstLinkHandle, (void*)&initargs) != 0)
			printf("Dstlinkhandle create failed!\r\n");

/*
	if (ret = pthread_create(&baselink, NULL, BaseStationLinkHandle, (void*)&initargs) != 0)
			printf("Basestation link create failed\r\n");
*/
	
	/*exit process*/
	while (TRUE) {
		char a ;
		scanf("%c", &a);
		if (a == 'a')
				exit(0);
	};

	fclose(pcfgfile);
	return 0;
}

