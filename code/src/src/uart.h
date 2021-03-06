/*
 * uart.h - uart configration interface 
 */

#include <stdio.h>
#include <stdlib.h>     /*��׼�����ⶨ��*/  
#include <unistd.h>     /*Unix ��׼��������*/  
#include <sys/types.h>   
#include <sys/stat.h>    
#include <fcntl.h>
#include <termios.h>
#include <fcntl.h>      /*�ļ����ƶ���*/  
#include <sys/select.h>

unsigned char Uart_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

