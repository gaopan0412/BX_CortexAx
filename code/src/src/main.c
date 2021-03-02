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
#include <sys/socket.h>
#include <netinet/in.h>

#define TRUE 1
#define FALSE 0

/*config file path*/
char pconfig[] = "./config.txt";

/*Destination station handler*/
void *DstLinkHandler(void *args)
{
	DstDataTask(args);
}

/*base station connet*/
void *BaseStationLinkHandler(void *args)
{
	BaseDataTask(args);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t dstlink, baselink;
	InitArgs_t initargs;
	FILE *pcfgfile = fopen(pconfig, "r");

	if (!pcfgfile)
	{
		printf("open config file failed!\r\n");
		return -1;
	}
	else
	{
		printf("open config file success\r\n");
	}

	get_initargs(pcfgfile, &initargs);

	if (ret = pthread_create(&dstlink, NULL, DstLinkHandler, (void *)&initargs) != 0)
		printf("Dstlinkhandle create failed!\r\n");

	/*
	if (ret = pthread_create(&baselink, NULL, BaseStationLinkHandler, (void*)&initargs) != 0)
			printf("Basestation link create failed\r\n");
  */

	//TODO:
	/*TODO: exit process and thread monitor*/
	while (TRUE)
	{
		char a;
		scanf("%c", &a);
		if (a == 'c')
			exit(0);
	};

	fclose(pcfgfile);
	return 0;
}
