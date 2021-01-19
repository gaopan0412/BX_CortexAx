/*
 *File - client.h 
 *Description: for implement client connection
 *Author: gaopan
 */



#ifndef __CLIENT_H
#define __CLIETN_H

#include "init.h"


int Tcp_ConnectDst(InitArgs_t* paddr, int sockfd);
int Tcp_ConnectBase(InitArgs_t* paddr, int sockfd);


#endif































