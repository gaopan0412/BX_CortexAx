/*
 * init.h - define some base config type
 */
#ifndef __INIT_H 
#define __INIT_H

/*for file pointer*/
#include <stdio.h>
#include "types.h"

/*routine log level*/
#define LEVEL0   0   /*no log*/
#define LEVEL1   1   /*for terminal*/
#define LEVEL2   2   /*for file*/
#define LEVEL3   3   /*file and terminal*/
#define LEVEL4   4   /*socket*/

/*log level switch*/
#define LOGLEVEL  LEVEL3

/*local define config*/
#define __LOCAL_SOL__
#define __DIFF_NTRIP__
#define __DIFF_TCP__
#define __DIFF_UART__   /*means diff data from uart1*/

#define ARGSLEN 32
/*the function return type*/
typedef enum _RetType { ERR = -1, SUCCESS = 0 } RetType;

/*need consider args lenth*/
typedef struct _InitArgs_t{ 
		char mode[ARGSLEN];		/*solution by terminal orserver*/ 
		char save[ARGSLEN];		/*save rawdata or not*/	
		char format[ARGSLEN];   /*out put message formatnmea or gins*/ 
		char dstip[ARGSLEN];    /*destionaton ipaddess*/  
	  char dstport[ARGSLEN];  /*destionation port*/ 
		char baseip[ARGSLEN];   /*base station ipdadress*/
		char baseport[ARGSLEN]; /*base station port*/
		char logpath[ARGSLEN];	/*this is routine log*/
		char rawpath[ARGSLEN];	/*for raw data store*/
		char SN[ARGSLEN];       /*device id*/
		int rawdatsave_flag;    /*save raw data flags*/
		//int dstfd;
		//int basefd;
}InitArgs_t;

/*
typedef volatile  IO;
typedef volatile  _IO;
typedef volatile  __IO;
typedef volatile  __IO__;
*/
int Debugs(uint8_t level, char* fmt, ...);
int Debug(uint8_t level, char* info);
RetType get_initargs(FILE* pcfgfile, InitArgs_t* arg);
#endif

