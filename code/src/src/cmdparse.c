/*
 * cmdparse.c - local or romate cmdline parse  interface
 */

#include <stdio.h>
#include <string.h>

void* local_cmd_process(char* cmd)
{
		if (strlen(cmd))
			printf("%s", (char*)cmd);
}



void remote_cmd_process(void* cmd)
{

	
}