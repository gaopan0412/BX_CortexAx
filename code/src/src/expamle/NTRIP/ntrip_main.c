#include"ntrip.h"
#include<unistd.h>


/*
	login format
	id:pwd@ipaddr:port:mntpt
*/

char login_cmcc[] = "abcf352:9mv2ebyj@117.135.142.201:8001/RTCM33_GRCE";

int main()
{
    stream_t stream;
    //char s[] = "CUMTBZJ:ZJ19951110@112.65.161.226:2101/CUT00";
   
	strinit(&stream);
    stropen(&stream, 7, STR_MODE_R, login_cmcc);
    unsigned char str[60];
    int n;
    while (1)
    {
        n=0;
        n=strread(&stream, str, 50);
        if(n){
            for(int i=0;i<n;i++){
                printf("%02x",str[i]);
            }
            printf("\n");
            fflush(stdout);
        }
        usleep(1000);
    }
    return 0;
}
