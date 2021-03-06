/*
 * cmdparse.h - local or romate cmdline parse  interface
 */

#ifndef __CMDPARSER_H
#define __CMDPARSER_H

/*local cmd process*/
void* local_cmd_process(char* cmd);
void remote_cmd_process(void* cmd);
#endif