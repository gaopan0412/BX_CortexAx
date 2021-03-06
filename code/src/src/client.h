/*
 *File - client.h 
 *Description: for implement client connection
 *Author: gaopan
 */



#ifndef __CLIENT_H
#define __CLIETN_H


//#define True (1)
//#define False (0)

#define OUTTIME (0)
#define MAXLEN (1024 * 1)

/*destination link task interface*/
void DstDataTask(void* args);
/*local solutions base staion interface*/
void BaseDataTask(void* args);

#endif































