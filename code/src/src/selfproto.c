/*
 * selfproto.c - implement self protocal 
 * -------------------------------------
 * format:
 * AA LENTH  PAYLOAD CRC16 55
 */
#include "selfproto.h"
#include "init.h"

static int set_msg_head()
{

  return 0;
}

static int set_msg_lenth()
{
  return 0;
}

static int set_msg_payload()
{
  return 0;
}

static int cal_crc16()
{

  return 0;
}

static int set_msg_tail()
{
  return 0;
}


int get_device_sn(InitArgs_t* args, uint8_t* sn_msg)
{

  return 0;
}

int self_proto_msg(uint8_t* in_msg, uint16_t in_lenth)
{


  return 0;
}
