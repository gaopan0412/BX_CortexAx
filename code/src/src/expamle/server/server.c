/* 
 * for implement server 
 * */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
 
//int open(const char *pathname, int flags);
//int open(const char *pathname, int flags, mode_t mode);

#define N 1204
typedef struct sockaddr SA;
char ip[] = "172.21.0.12";

typedef void (*callback_t)(int fd);
/*
void handle(int fd)
{
	printf("fd:%d\n", fd);
	static int count = 0;
	while(1)
	{
		send(fd,  "this is master\r\n", strlen("this is master\r\n"), 0);
		printf("count:%d\n", count++);
		sleep(1);			
	}
}
void slot_func(int sockfd, callback_t call_back_func)
{
	call_back_func(sockfd);
}
*/

void* thread_handle(void* arg)
{
	int sockfd = *(int*)arg;
	char recvbuf[1024] = {0};
	char sendbuf[] = "this is sever\r\n";
	pthread_detach(pthread_self());

	while(1)
	{
	   int len = recv(sockfd, recvbuf, 1024, 0);
	   if (len >0)
		{
			printf("%s", recvbuf);
			if (send(sockfd, sendbuf, strlen(sendbuf), 0) < 0)
			{
				break;
			}
		} else if ( len < 0)
		{
			break;
		} else 
		{
		 	;
		}
	
		sleep(1);
	}

	close(sockfd);
	printf("---client quit\n");	
	pthread_exit(arg);
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Using: %s port\n", argv[0]);
		exit(-1);
	}

	char sendbuf[20] = "====>master\n";	
	unsigned int count = 0;
	int listenfd, connfd;
	struct sockaddr_in myaddr, peeraddr;
	char buf[N] = {0};
	
	socklen_t mylen, peerlen;
	mylen = sizeof(myaddr);
	peerlen = sizeof(peeraddr);
	int imufd = open("./imudata.bin", O_RDWR|O_CREAT, 0666);
	
	
	/*create socket*/
	if ((listenfd = (socket(AF_INET, SOCK_STREAM, 0))) < 0) {
		perror("socket");
		exit(-1);
	}

	bzero(&myaddr, mylen);
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(atoi(argv[1]));
	inet_aton(ip, &myaddr.sin_addr);

	if (bind(listenfd, (SA*)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind");
		exit(-1);
	}
	
	listen(listenfd, 5);

	while(1)
	{
		if (-1 == (connfd = accept(listenfd, (SA*)&peeraddr, &peerlen))) 
		{
			printf("[accept] failed!\n");
			//exit(-1);
		} else 
		{	
			printf("connet from %s:%d sockfd:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), connfd);		
			//slot_func(connfd, handle);
			//create a thread for process
			pthread_t threadid;
			pthread_create(&threadid, NULL, thread_handle, &connfd);
			
		}
	}
/*	
	while(1) {
		int n  = recv(connfd, buf, N, 0);
		if ( n > 0) {
			send(connfd, sendbuf, strlen(sendbuf), 0);
			printf("lenth:%d  %s", n, (char*)buf);
		}

		if (!strncmp(buf, "quit", 4)) {
			send(connfd, "close connetion", strlen("close connetion"), 0);
		}
		sleep(1);
	}
*/
	close(connfd);
	close(listenfd);
	exit(0);
}
