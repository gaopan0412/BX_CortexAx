#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

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


int main(int argc, char* argv[])
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
		if (send(sockfd, gngga, strlen(gngga), OUTTIME) < 0)
		{
			while(connect(sockfd, (struct sockaddr*)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				printf("connect server....\n");
				sleep(1);
			}
		}

		sleep(1);
/*
		int len = recv(sockfd, buffer, MAXLEN, OUTTIME);
		if (len > 0)
		{
			printf("[recv]lenth:%d %s",len,  buffer);
		} else if (len < 0)
		{	
			while(connect(sockfd, (struct sockaddr*)(&ipaddr), sizeof(struct sockaddr)) < 0)
			{
				printf("connect server\n");
				sleep(1);
			}
		} else {};
*/
	}	

	free(buffer);
	return 0;
}
