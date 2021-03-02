#include "uart.h"
#include "init.h"

/*
Func: Uart_Init
Description: ���ڳ�ʼ��
Input:
			   fd:		�����ļ�������
			speed:      �����ٶ�
		flow_ctrl:		����������
		 databits:		����λ   ȡֵΪ 7 ����8
		 stopbits:		ֹͣλ   ȡֵΪ 1 ����2
		   parity:		Ч������ ȡֵΪN,E,O,,S
Others:
Author: 
*/
unsigned char Uart_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{

	int   i;
	int   speed_arr[] = { B460800, B115200, B19200, B9600, B4800, B2400, B1200, B300 };
	int   name_arr[] = { 460800, 115200,  19200,  9600,  4800,  2400,  1200,  300 };

	struct termios options;

	/*tcgetattr(fd,&options)�õ���fdָ��������ز������������Ǳ�����options,�ú��������Բ��������Ƿ���ȷ���ô����Ƿ���õȡ������óɹ�����������ֵΪ0��������ʧ�ܣ���������ֵΪ1.
	*/
	if (tcgetattr(fd, &options) != 0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}

	//���ô������벨���ʺ����������  
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++)
	{
		if (speed == name_arr[i])
		{
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	//�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���  
	options.c_cflag |= CLOCAL;
	//�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������  
	options.c_cflag |= CREAD;

	//��������������  
	switch (flow_ctrl)
	{

	case 0://��ʹ��������  
		options.c_cflag &= ~CRTSCTS;
		break;

	case 1://ʹ��Ӳ��������  
		options.c_cflag |= CRTSCTS;
		break;
	case 2://ʹ������������  
		options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}
	//��������λ  
	//����������־λ  
	options.c_cflag &= ~CSIZE;
	switch (databits)
	{
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return (FALSE);
	}
	//����У��λ  
	switch (parity)
	{
	case 'n':
	case 'N': //����żУ��λ��  
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O'://����Ϊ��У��      
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E'://����ΪżУ��    
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S': //����Ϊ�ո�   
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return (FALSE);
	}
	// ����ֹͣλ   
	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB; break;
	case 2:
		options.c_cflag |= CSTOPB; break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return (FALSE);
	}

	//�޸����ģʽ��ԭʼ�������  
	options.c_oflag &= ~OPOST;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);  

	//���õȴ�ʱ�����С�����ַ�  
	options.c_cc[VTIME] = 1; /* ��ȡһ���ַ��ȴ�1*(1/10)s */
	options.c_cc[VMIN] = 1; /* ��ȡ�ַ������ٸ���Ϊ1 */
	//for recv '\r' 
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	//�����������������������ݣ����ǲ��ٶ�ȡ ˢ���յ������ݵ��ǲ���  
	tcflush(fd, TCIFLUSH);

	//�������� (���޸ĺ��termios�������õ������У�  
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("com set error!\n");
		return (FALSE);
	}
	return (TRUE);
}


