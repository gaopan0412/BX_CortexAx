/*
 *File - client.c 
 *Description: for implement client connection
 *Author: gaopan
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
#include "init.h"

/*
 * Tcp_ConnectDst 
 * paddr   I  ipv4 addr and port 
 * sockfd  O  socket file description
 * ret : SUCCESS  0
 * 		 FAILED  -1
 */

int Tcp_ConnectDst(InitArgs_t* paddr, int sockfd)
{
	int ret = 0;
	int addlenth = strlen(paddr->dstip);
	unsigned int port = atoi(paddr->dstport);
	char addr[32]={0};

	memcpy(addr, paddr->dstip, addlenth);

	if (!paddr) {
		Debugs(LOGLEVEL, "[Error]: Dst addr and port is null\r\n");
		return -1;
	}	

	struct sockaddr_in tcpaddr;
	tcpaddr.sin_family = AF_INET;
	paddr->dstfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0 ) {
		Debugs(LOGLEVEL, "[Error]:Dst socket create failed!\r\n");
	}

	//Debug(LEVEL3, "ip:%s  port:%u\r\n", addr, port);
   
//	bzero(&tcpaddr, sizeof(tcpaddr));
	tcpaddr.sin_port = htons(port);
    ret = inet_pton(AF_INET, addr, &tcpaddr.sin_addr.s_addr);
	if ( ret < 0) 
		Debugs(LOGLEVEL, "inet_pton error\r\n");
	
	ret = connect(paddr->dstfd, (struct sockaddr*)&tcpaddr, sizeof(tcpaddr));
	if (ret < 0) {
		Debugs(LOGLEVEL, "[Error]:Dst connect failed!\r\n");
	} else {
		Debugs(LOGLEVEL, "Dst connect success\r\n");
	}

	return 0;
}

/*
 * Tcp_ConnectBase 
 * paddr   I  ipv4 addr and port 
 * sockfd  O  socket file description
 * ret : SUCCESS  0
 * 		 FAILED  -1
 */
int Tcp_ConnectBase(InitArgs_t* paddr, int sockfd)
{
	int ret = 0;
	unsigned int port = atoi(paddr->baseport);
	int addlenth = strlen(paddr->baseip);
	char addr[32] ={0};
	memcpy(addr, paddr->baseip, addlenth);

	
	if (!paddr) {
		Debugs(LOGLEVEL, "[Error]:Base addr and port is null\r\n");
		return -1;
	}	

	struct sockaddr_in tcpaddr;
	tcpaddr.sin_family = AF_INET;
	//sockfd = socket(AF_INET, SOCK_STREAM, 0);
	paddr->basefd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0 ) {
		Debugs(LOGLEVEL, "[Error]:Base socket create failed!\r\n");
	}

	//Debug(LEVEL3, "ip:%s  port:%u\r\n", addr, port);
   
	//bzero(&tcpaddr, sizeof(tcpaddr));
	tcpaddr.sin_port = htons(port);
    ret = inet_pton(AF_INET, addr, &tcpaddr.sin_addr.s_addr);
	if ( ret < 0) {
		Debugs(LOGLEVEL, "inet_pton error\r\n");
		return -1;
	}
	

	ret = connect(paddr->basefd, (struct sockaddr*)&tcpaddr, sizeof(tcpaddr));

	if (ret < 0) {
		Debugs(LOGLEVEL, "[Error]:Base connect failed!\r\n");
		return -1;
	} else {
		Debugs(LOGLEVEL, "Base connect success\r\n");
	}

	return 0;
}





















