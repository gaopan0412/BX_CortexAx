/*
 *init.c - implement parse config argumets func
 */

//#include <stdio.h> 
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "init.h"
#include "types.h"

#define LINE_MAX 1024

/*routine log file pointer*/
static FILE* glogfilep = NULL;

/*
 * set_default_args
 * args:  InitArgs_t* arg I/O was seted argument struct
 * return: ERR or SUCCESS
 * note:
 */

static RetType set_default_args(InitArgs_t* args)
{
	/*TODO*/
	return ERR;
}

/*
 * get_configargs: get parameters from config file
 * const* char* str  I parameters string
 * char* 		args O get string
 */
static RetType get_configargs(const char* str, char* args)
{
	const char *temp = str;
	int i = 0, flag = 0;
	
	if ( !str || !args) {
		Debugs(LOGLEVEL, "Error: args is NULL %s",  __func__);
		return ERR;
	}

	while ( *temp != '\0') {
		/*find the line tail*/	
		if (*temp == '\r' || *temp == '\n' || *temp == ' ')
			break;

		/*get paramtes*/
		if (flag)
			args[i++] = *temp;

		/*get start flag*/
		if (*temp == '=') {
			flag = 1;
		}

		temp++;
	}
	
	return SUCCESS;
}

/* parse_initargs 
 * args: char*       buf   I  string
 * 		 int    	 len   I  string lenth
 *		 InitArgs_t* arg   O  int argument stuct
 * return: ERR or SUCCESS
 * note:
 */
static RetType parse_initargs(const char* buf, int len, InitArgs_t* arg)
{
	if (!buf || ! arg) {
		Debugs(LOGLEVEL, "Error: char buffer is null or arg is null\r\n");
		return ERR;
	}	

	
	if (!strncmp(buf, "dstip", strlen("dstip"))) {		
		/*destination ip address*/
		get_configargs(buf, arg->dstip);
	
	} else if (!strncmp(buf, "dstport", strlen("dstport"))) {
		/*destination port*/	
		get_configargs(buf, arg->dstport);
	
	} else if (!strncmp(buf, "localsolution", strlen("localsolution"))) {
		/*solution mode terminal or server*/	
		get_configargs(buf, arg->mode);

	} else if (!strncmp(buf, "baseip", strlen("baseip"))) {
		/*base station ip address*/
		get_configargs(buf, arg->baseip);
	
	} else if (!strncmp(buf, "baseport", strlen("baseport"))) {
		/*base station port*/
		get_configargs(buf, arg->baseport);

	} else if (!strncmp(buf, "save", strlen("save"))) {
		/*save raw data or not*/
		get_configargs(buf, arg->save);
	
	} else if (!strncmp(buf, "format", strlen("format"))) {
		/*if solution at terminal output format NMEA or other*/
		get_configargs(buf, arg->format);

	} else if (!strncmp(buf, "logpath", strlen("logpath"))) {
		/*the routin run log*/
		get_configargs(buf, arg->logpath);

	} else if (!strncmp(buf, "rawpath", strlen("rawpath"))) {
		/*the raw data store path*/
		get_configargs(buf, arg->rawpath);

	} else if (!strncmp(buf, "SN", strlen("SN"))){
		get_configargs(buf, arg->SN);
	} else {

		/*TODO:Extern paramters*/
	}

	return SUCCESS;

}

/*
 * get_initargs
 * args:
 * 			in: pcfgfile the init config file pointer 
 * 			out: arg global config struct all config info
 * ret : RetType  get config file or not
 */
RetType get_initargs(FILE* pcfgfile, InitArgs_t* arg)
{
	char buff[LINE_MAX] = {0};

	if ( !pcfgfile || !arg) {
		Debugs(LOGLEVEL, "Error: pcfgfile or InitArgs NULL\r\n");
		return ERR;
	}
		
	glogfilep = fopen("./monitor_routine_log.txt", "w+");
	if (!glogfilep) {
		Debugs(LOGLEVEL, "Error: open log file failed!\r\n");
		return ERR;
	}

	memset(arg, 0 , sizeof(InitArgs_t));

	while (!feof(pcfgfile)) {
	
	 	fgets(buff, LINE_MAX, pcfgfile);
		parse_initargs(buff, strlen(buff), arg);
	}

	return SUCCESS;
}
/* 
 * close routine log file
 */
static void close_logfile(void)
{
	fclose(glogfilep);
}

/* 
 * get systecm local time
 */
static int get_systime(void)
{
	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);
	Debugs(LOGLEVEL, "%4d/%02d/%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	return 0;
}


/* Debugs
 *      LEVEL0: no debug info
 *      LEVEL1: for terminal
 *      LEVEL2: for file
 *      LEVEL3: for terminal and file
 *      LEVEL4: socket communicate
 */
int Debugs(uint8_t level, char* fmt, ...)
{
	/*need consider this buffer size*/
	char message[1024] ={0};
	char timestr[256] = {0};

 	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);

	if (!fmt) { 
		printf("Error: no input debug message\r\n");
		return -1;	
	}

	/*debug info add time string*/
	sprintf(timestr, "[%4d-%02d-%02d %02d:%02d:%02d] ", t->tm_year + 1900,  t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	va_list args;
	va_start(args, fmt); /*buffer args*/
	vsprintf(message, fmt, args);
	
	switch(level) {
	 case LEVEL0:
				/*anything not output*/
				break;
	 case LEVEL1:
				printf("%s%s", timestr, message);
				break;
	 case LEVEL2:
				fprintf(glogfilep, "%s%s", timestr, message);
				break;
     case LEVEL3:
				printf("%s%s", timestr, message);
				fprintf(glogfilep, "%s%s", timestr, message);
				break;
	case LEVEL4:
				/*TODO:socket debug info*/
				break;
	default:
				printf("ERR: level error\r\n");
				//return -1;
	}
		
	fflush(glogfilep);
	va_end(args);
	return 0;
}

/*
 * config_info_parse(InitArgs_t* args)
 * IN : args
 * OUT: args
 * brif:  convert string init arguments to int
 * ret : false get unormal paramters
 * 				true get real paramtes
 */

static int  config_info_parse(InitArgs_t* args)
{
	if (!strcmp(args->save, "yes") || !strcmp(args->save, "YES"))
	{
			args->rawdatsave_flag = TRUE;
	} else 
	{
			args->rawdatsave_flag = FALSE;
	}

	//TODO: others arguments convert
	return 0;
}