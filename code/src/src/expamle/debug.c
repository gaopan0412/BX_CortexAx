#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

FILE* log = NULL;

int debug(char* fmt, ...)
{
	char buff[1024] ={0};

	va_list args;
	va_start(args, fmt);

	vsprintf((char*)buff, fmt, args);
	printf("%s", (char*)buff);
	fprintf(log, "%s", (char*)buff);

/*

	vprintf(fmt, args);
	fprintf(log, fmt , args);
*/
	va_end(args);

	return 0;
}

#if 0
int get_mem(char* p, int n)
{
	p = (char*)malloc(sizeof(char)*n);
	return 0;
}
#endif

char* get_mem(int n)
{
	return (char*)malloc(sizeof(char)*n);
}

int main(void)
{
	log = fopen("./log", "w+");
	if(!log) 
	{
		printf("[error]open file fail\n");
		return -1;
	} else 
	{
		printf("open file success\n");
	}
	
	debug("[debug]:%s %d\r\n", "hello", 2);
	debug("[debug]:%s %f\r\n", "hello", 3.12);
	debug("[debug]:hello world\r\n");

	char* temp = NULL; 
	temp = get_mem(10);
	//memcpy(temp, "ddd", 3);
/*
	if (!temp)
		printf("[ddd]pointer is NULL\r\n");
	else 
		printf("pointer is %p\r\n", temp);
	
	memcpy(temp, "ddd", 3);
	printf("%s", temp);
*/
	fclose(log);
	return 0;
}

