#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define portnum 8499
char ipaddress[] = "182.92.241.63";

int main(int argc, char* argv[])
{
	struct sockaddr_in ipaddr;
	char buffer[128] = {0};
	
	int sockfd;
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
	ipaddr.sin_port = htons(portnum);
	ipaddr.sin_addr.s_addr = inet_addr(ipaddress);
	
	if (connect(sockfd, (struct sockaddr*)(&ipaddr), sizeof(struct sockaddr)) < 0)
	{
		printf("connect error\n");
		exit(-1);
	} else 
	{
		printf("open server success!\n");
		close(sockfd);
	}
	
	return 0;
}
