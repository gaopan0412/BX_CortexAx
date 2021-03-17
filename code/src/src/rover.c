/*
 * rover.c - local rover data process
 */

#include <string.h>
#include <pthread.h>
#include "uart.h"
#include "rover.h"
#include "init.h"

#define RAWDATALEN (4096)
static char gnssport[] = "/dev/ttyS3";

/* rover_init
 * args: none
 * ret : uart fd
 * brif: init rover data uart
 */
static int rover_init(void)
{
  /*open uart and init*/
  int rover_data_fd = open(gnssport, O_RDWR);
  int ret = 0;
  if (rover_data_fd < 0)
  {
    Debugs(LOGLEVEL, "open rover /dev/ttyS3 uart failed!\n");
    return ERR;
  }
  else
  {
    ret = Uart_Init(rover_data_fd, 115200, 0, 8, 1, 'N');
  }
  // need uart return status
  //Todo: config gnss board
  if (ret == TRUE)
  {
    return rover_data_fd;
  }
  else
  {
    Debugs(LOGLEVEL, "Init uart faile!\n");
    return ERR;
  }
}

static int gnss_config(int gnss_data_fd)
{

  return SUCCESS;
}

void *RoverDataTask(void *args)
{
  InitArgs_t* init_args = (InitArgs_t*)args;
  pthread_detach(pthread_self());
  int rawdatafd = 0;


  Debugs(LOGLEVEL,"lenth:%d func:%s %s %s\n", __LINE__, __func__, init_args->rawpath, init_args->save);

  int roverfd = rover_init();
  if (roverfd < 0)
  {
    Debugs(LOGLEVEL, "uart open failed!\n");
    pthread_exit(args);
  }

  uint8_t* rawdatabuf = (uint8_t*) malloc(sizeof(uint8_t)*RAWDATALEN);
  if (!rawdatabuf) 
  {
    Debugs(LOGLEVEL, "raw data buffer malloc failed! func:%s line;%d\n", __func__, __LINE__);
    pthread_exit(args);
  } else
  {
    memset(rawdatabuf, 0, sizeof(uint8_t)*RAWDATALEN);
  }
  
  Debugs(LOGLEVEL, "flag:%d\n", init_args->rawdatsave_flag);

  if (init_args->rawdatsave_flag)
  {
    //Debugs(LOGLEVEL, "[raw data]raw rover data:%s\n", init_args->rawpath);
    //TODO:open raw data file
  }

 
  while (TRUE)
  {
    long lenth = read(roverfd,rawdatabuf, RAWDATALEN);
    if ( 0 < lenth)
    {
      //input buffer
    }   

    if (init_args->rawdatsave_flag)
    {
      

    }
    //sleep(1);
    //Debugs(LOGLEVEL, "this is rover\n");
  }

  pthread_exit(args);
}

