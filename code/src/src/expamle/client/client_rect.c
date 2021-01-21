#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

//#define __CMCC__
#define portnum 8520
#define CMCCPORT 8001


#define True  (1)
#define False (0)

#define OUTTIME (0)
#define MAXLEN  (1024*1)


char cmcc_ip[] = "117.135.142.201";
char ipaddress[] = "81.70.3.131";
char gngga[] = "$GNGGA,072844.000,3945.66037,N,11619.73066,E,1,26,0.55,40.5,M,-8.3,M,,*59\r\n";

int thread_flag = 0;

void* thread_handler(void* args)
{
	int sockfd;
	struct sockaddr_in ipaddr;
	char* buffer= (char*) malloc(sizeof(char)*MAXLEN);
	if (!buffer)
	{
		printf("memory malloc failed!\n");
		exit(-1);
	} else
	{
		printf("malloc success!\n");
		memset(buffer, 0, sizeof(char)*MAXLEN);
	}
	
	pthread_detach(pthread_self());

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("creat socket error!\n");
		exit(-1);
	} else
	{
		printf("creat socket success!\n");
	}

	bzero(&ipaddr, sizeof(struct sockaddr_in));
	ipaddr.sin_family = AF_INET;
#ifdef __CMCC__ 
	ipaddr.sin_port = htons(CMCCPORT);
	ipaddr.sin_addr.s_addr = inet_addr(cmcc_ip);
	printf("cmcc test\n");
#else
	ipaddr.sin_port = htons(portnum);
	ipaddr.sin_addr.s_addr = inet_addr(ipaddress);
	printf("TenceYun test\n");
#endif	
	if (connect(sockfd, (struct sockaddr*)(&ipaddr), sizeof(struct sockaddr)) < 0)
	{
		printf("connect error\n");
		exit(-1);
	} else 
	{
		printf("open server success! fd:%d\n", sockfd);
	}

	while (True)
	{
		if (send(sockfd, gngga, strlen(gngga), MSG_NOSIGNAL) < 0)
		{

			printf("connect failed!\n");

			if(!close(sockfd))
	        	sockfd = socket(AF_INET, SOCK_STREAM, 0);
			//Todo: close failed 
			while(connect(sockfd, (struct sockaddr*)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				printf("send reconnect server...\n");
				usleep(200000);
			}
			printf("send reconnect success!\n");
			continue;
		}

		sleep(1);
		int len = 0;		
		if ((len = recv(sockfd, buffer, MAXLEN, MSG_NOSIGNAL)) < 0) 
		{	
			if(!close(sockfd))
	        	sockfd = socket(AF_INET, SOCK_STREAM, 0);
			//Todo: close failed!
			printf("recv reconnect failed!\n");
			while(connect(sockfd, (struct sockaddr*)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				printf("recv reconnect server...\n");
				usleep(200000);
			}

			printf("recv reconnect success!\n");
		} else if(len > 0)
		{
			printf("len:%d %s", len, buffer);
		} else {};
	}	

	close(sockfd);
	free(buffer);
	thread_flag = 0;//meas thread exit
	pthread_exit(args);

}

int main(int argc, char* argv[])
{
	while(True)
	{
		if (!thread_flag)
		{
			pthread_t thread_id;
			if (pthread_create(&thread_id, NULL, thread_handler,  NULL) < 0) 
			{
				printf("thread_create error\n");
				continue;
			} else 
			{
				printf("thread create success\n");
				thread_flag = 1;
			}
		} else {
			sleep(1);
			printf("thread monitor\n");
		}

	}

	return 0;
}
