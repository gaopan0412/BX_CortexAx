/*
 *File - client.h 
 *Description: for implement client connection
 *Author: gaopan
 */



#ifndef __CLIENT_H
#define __CLIETN_H

#include "init.h"

#define True (1)
#define False (0)

#define OUTTIME (0)
#define MAXLEN (1024 * 1)


//int Tcp_ConnectDst(InitArgs_t* paddr, int sockfd);
//int Tcp_ConnectBase(InitArgs_t* paddr, int sockfd);
void DstDataTask(void* args);
void BaseDataTask(void* args);

#endif































