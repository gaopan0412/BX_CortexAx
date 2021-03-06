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
#include "types.h"
#include "rover.h"
#include "cmdparse.h"

/*config file path*/
char pconfig[] = "./config.txt";

/*Destination station handler*/
static void *DstLinkHandler(void *args)
{
	DstDataTask(args);
}

/*Rover station connet*/
static void *RoverHandler(void *args)
{
	RoverDataTask(args);
}

/*base station connet*/
static void *BaseStationLinkHandler(void *args)
{
	BaseDataTask(args);
}


int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t dstlink, baselink, rover;
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

 if (ret = pthread_create(&rover, NULL, RoverHandler, (void*)&initargs) != 0)
			printf("Rover thread create failed\r\n");

 if (ret = pthread_create(&dstlink, NULL, DstLinkHandler, (void *)&initargs) != 0)
		printf("Dstlinkhandle create failed!\r\n");

#ifdef __LOCAL_SOL__
 //this thread needn't , solution in server 
 if (ret = pthread_create(&baselink, NULL, BaseStationLinkHandler, (void*)&initargs) != 0)
	  printf("Basestation link create failed\r\n");
#endif

	//TODO:
	/*TODO: exit process and thread monitor*/
	while (TRUE)
	{
		char cmd[] = "\0";
		scanf("%s", cmd);
		if (strlen(cmd)) 
		{
			if (strstr(cmd, "quit"))
			{	
				Debugs(LOGLEVEL, "get quit cmd and exit process\n");
				exit(0);
			}
			else 
			{
				local_cmd_process(cmd);
			}
		}

		Debugs(LOGLEVEL,"SN:%s\n", initargs.SN);
		Debugs(LOGLEVEL,"RAWDATA:%s\n", initargs.rawpath);
		//TODO: others thread monitor
		sleep(1);
	};

	fclose(pcfgfile);
	return 0;
}
