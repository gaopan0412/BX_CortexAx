#include <stdio.h>
#include <stdlib.h>     /*标准函数库定义*/  
#include <unistd.h>     /*Unix 标准函数定义*/  
#include <sys/types.h>   
#include <sys/stat.h>    
#include <fcntl.h>
#include <termios.h>
#include <fcntl.h>      /*文件控制定义*/  
#include <sys/select.h>

unsigned char Uart_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

