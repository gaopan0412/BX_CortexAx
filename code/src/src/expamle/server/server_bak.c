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
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
 
//int open(const char *pathname, int flags);
//int open(const char *pathname, int flags, mode_t mode);

#define N 1204
typedef struct sockaddr SA;
char ip[] = "172.21.0.12";


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
	if (-1 == (connfd = accept(listenfd, (SA*)&peeraddr, &peerlen))) {
		perror("accept");
		exit(-1);
	}
	
	printf("connet from %s:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
	
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

	close(connfd);
	close(listenfd);
	exit(0);
}
