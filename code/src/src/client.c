/*
 * File - client.c 
 * Description: for implement client connection
 * Author: gaopan
 *
 */

#include <unistd.h>
//#include <socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "init.h"
#include "client.h"

/*
 * DstDataTask 
 * args: none
 * ret : none
 * brif: solution data destination server link thread
 */
void DstDataTask(void *args)
{

	int sockfd;
	struct sockaddr_in ipaddr;
	char *buffer = (char *)malloc(sizeof(char) * MAXLEN);
	InitArgs_t *initarg = (InitArgs_t *)args;
	int portnum = atoi(initarg->dstport);
	char gngga[] = "$GNGGA,072844.000,3945.66037,N,11619.73066,E,1,26,0.55,40.5,M,-8.3,M,,*59\r\n";

	Debugs(LOGLEVEL, "destination port:%d\n", portnum);

	if (!buffer)
	{
		Debugs(LOGLEVEL, "%s", "destination memory malloc failed!\n");
		exit(-1);
	}
	else
	{
		Debugs(LOGLEVEL, "destination malloc success!\n");
		memset(buffer, 0, sizeof(char) * MAXLEN);
	}

	pthread_detach(pthread_self());

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		Debugs(LOGLEVEL, "destination creat socket error!\n");
		exit(-1);
	}
	else
	{
		Debugs(LOGLEVEL, "destination creat socket success!\n");
	}

	bzero(&ipaddr, sizeof(struct sockaddr_in));
	ipaddr.sin_family = AF_INET;
	ipaddr.sin_port = htons(portnum);
	ipaddr.sin_addr.s_addr = inet_addr(initarg->dstip);
	//Debugs("TenceYun test\n");

	if (connect(sockfd, (struct sockaddr *)(&ipaddr), sizeof(struct sockaddr)) < 0)
	{
		Debugs(LOGLEVEL, "destination connect error\n");
		exit(-1);
	}
	else
	{
		Debugs(LOGLEVEL, "destination open server success! fd:%d\n", sockfd);
	}


	//TODO: send rover id to destination server

	while (TRUE)
	{

		//(1) get rover raw data
	#ifdef __LOCAL_SOL__
		//solution
		//get base data from base buffer
	#else 
		//self prorocal 

	#endif

		if (send(sockfd, gngga, strlen(gngga), MSG_NOSIGNAL) < 0)
		{

			Debugs(LOGLEVEL, "destination connect failed!\n");

			if (!close(sockfd))
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
			//Todo: close failed
			while (connect(sockfd, (struct sockaddr *)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				Debugs(LOGLEVEL, "destination send reconnect server...\n");
				usleep(200000);
			}
			Debugs(LOGLEVEL, "destination send reconnect success!\n");
			continue;
		}

		sleep(1);
		int len = 0;
		if ((len = recv(sockfd, buffer, MAXLEN, MSG_NOSIGNAL)) < 0)
		{
			//Todo parse remote commander
			if (!close(sockfd))
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
			//Todo: close failed!
			Debugs(LOGLEVEL, "destination recv reconnect failed!\n");
			while (connect(sockfd, (struct sockaddr *)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				Debugs(LOGLEVEL, "destination recv reconnect server...\n");
				usleep(200000);
			}

			Debugs(LOGLEVEL, "destination recv reconnect success!\n");
		}
		else if (len > 0)
		{
			Debugs(LOGLEVEL, "destination len:%d %s", len, buffer);
		}
		else
		{
		};
	}

	close(sockfd);
	free(buffer);
	//thread_flag = 0;//meas thread exit
	pthread_exit(args);
}

/*
 *  BaseDataTask
 *  args: none
 * 	ret : none
 * 	brif: Base station data process thread 
 */
void BaseDataTask(void *args)
{

	int sockfd;
	struct sockaddr_in ipaddr;
	char *buffer = (char *)malloc(sizeof(char) * MAXLEN);
	InitArgs_t *initarg = (InitArgs_t *)args;
	int portnum = atoi(initarg->baseport);
	char gngga[] = "$GNGGA,072844.000,3945.66037,N,11619.73066,E,1,26,0.55,40.5,M,-8.3,M,,*59\r\n";

	Debugs(LOGLEVEL, "basestion port:%d\n", portnum);

	if (!buffer)
	{
		Debugs(LOGLEVEL, "basestion memory malloc failed!\n");
		exit(-1);
	}
	else
	{
		Debugs(LOGLEVEL, "basestion malloc success!\n");
		memset(buffer, 0, sizeof(char) * MAXLEN);
	}

	pthread_detach(pthread_self());

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		Debugs(LOGLEVEL, "basestion creat socket error!\n");
		exit(-1);
	}
	else
	{
		Debugs(LOGLEVEL, "basestion creat socket success!\n");
	}

	bzero(&ipaddr, sizeof(struct sockaddr_in));
	ipaddr.sin_family = AF_INET;
	ipaddr.sin_port = htons(portnum);
	ipaddr.sin_addr.s_addr = inet_addr(initarg->baseip);
	//Debugs("TenceYun test\n");

	if (connect(sockfd, (struct sockaddr *)(&ipaddr), sizeof(struct sockaddr)) < 0)
	{
		Debugs(LOGLEVEL, "basestion connect error\n");
		exit(-1);
	}
	else
	{
		Debugs(LOGLEVEL, "basestion open server success! fd:%d\n", sockfd);
	}

	while (TRUE)
	{

	/*(1) get base data*/
		if (send(sockfd, gngga, strlen(gngga), MSG_NOSIGNAL) < 0)
		{

			Debugs(LOGLEVEL, "basestion connect failed!\n");

			if (!close(sockfd))
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
			//Todo: close failed
			while (connect(sockfd, (struct sockaddr *)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				Debugs(LOGLEVEL, "basestion send reconnect server...\n");
				usleep(200000);
			}
			Debugs(LOGLEVEL, "basestion send reconnect success!\n");
			continue;
		}

		sleep(1);
		int len = 0;
		if ((len = recv(sockfd, buffer, MAXLEN, MSG_NOSIGNAL)) < 0)
		{
			if (!close(sockfd))
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
			//Todo: close failed!
			Debugs(LOGLEVEL, "basestion recv reconnect failed!\n");
			while (connect(sockfd, (struct sockaddr *)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				Debugs(LOGLEVEL, "basestion recv reconnect server...\n");
				usleep(200000);
			}

			Debugs(LOGLEVEL, "basestion recv reconnect success!\n");
		}
		else if (len > 0)
		{
			Debugs(LOGLEVEL, "basestion len:%d %s", len, buffer);
		}
		else
		{
			//means send success
		};
	}

	close(sockfd);
	free(buffer);
	//thread_flag = 0;//meas thread exit
	pthread_exit(args);
}