#ifndef NTRIP_H
#define NTRIP_H
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include<ctype.h>
#include<pthread.h>
#define lock_t      pthread_mutex_t
#define MAXSTRPATH  1024                /* max length of stream path */
#define MAXSTRMSG   1024                /* max length of stream message */
#define STR_MODE_R  0x1                 /* stream mode: read */
#define STR_MODE_W  0x2                 /* stream mode: write */
#define STR_MODE_RW 0x3                 /* stream mode: read/write */

#define STR_NTRIPCLI 7                  /* stream type: NTRIP client */

typedef struct {        /* stream type */
    int type;           /* type (STR_???) */
    int mode;           /* mode (STR_MODE_?) */
    int state;          /* state (-1:error,0:close,1:open) */
    unsigned int inb,inr;   /* input bytes/rate */
    unsigned int outb,outr; /* output bytes/rate */
    unsigned int tick_i; /* input tick tick */
    unsigned int tick_o; /* output tick */
    unsigned int tact;  /* active tick */
    unsigned int inbt,outbt; /* input/output bytes at tick */
    lock_t lock;        /* lock flag */
    void *port;         /* type dependent port control struct */
    char path[MAXSTRPATH]; /* stream path */
    char msg [MAXSTRMSG];  /* stream message */
} stream_t;
extern int strread(stream_t *stream,unsigned char *buff,int n);
extern void strinit(stream_t *stream);
extern int stropen(stream_t *stream, int type, int mode, const char *path);
#endif