#include <ctype.h>
#include"ntrip.h"
#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifndef CRTSCTS
#define CRTSCTS  020000000000
#endif
#include <errno.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#define socket_t            int
#define closesocket         close
#define NTRIP_MAXRSP        32768       /* max size of ntrip response */
#define NTRIP_MAXSTR        256         /* max length of mountpoint string */
#define NTRIP_RSP_ERROR     "ERROR"     /* ntrip response: error */
#define NTRIP_RSP_OK_SVR    "OK\r\n"    /* ntrip response: server */
#define MAXSTATMSG          32          /* max length of status message */
#define NTRIP_AGENT         "RTKLIB"
#define NTRIP_RSP_OK_CLI    "ICY 200 OK\r\n" /* ntrip response: client */
#define NTRIP_RSP_SRCTBL    "SOURCETABLE 200 OK\r\n" /* ntrip response: source table */
#define NTRIP_RSP_HTTP      "HTTP/"     /* ntrip response: http */
#define NTRIP_CLI_PORT      2101        /* default ntrip-client connection port */
#define NTRIP_SVR_PORT      80          /* default ntrip-server connection port */
#define thread_t    pthread_t
#define lock_t      pthread_mutex_t
#define initlock(f) pthread_mutex_init(f,NULL)
#define lock(f)     pthread_mutex_lock(f)
#define unlock(f)   pthread_mutex_unlock(f)
static int toinact  =10000; /* inactive timeout (ms) */
static int buffsize =32768; /* receive/send buffer size (bytes) */
static int ticonnect=10000; /* interval to re-connect (ms) */
static char proxyaddr[256]=""; /* http/ntrip/ftp proxy address */
typedef struct {            /* tcp control type */
    int state;              /* state (0:close,1:wait,2:connect) */
    char saddr[256];        /* address string */
    int port;               /* port */
    struct sockaddr_in addr; /* address resolved */
    socket_t sock;          /* socket descriptor */
    int tcon;               /* reconnect time (ms) (-1:never,0:now) */
    unsigned int tact;      /* data active tick */
    unsigned int tdis;      /* disconnect tick */
} tcp_t;
typedef struct {            /* tcp cilent type */
    tcp_t svr;              /* tcp server control */
    int toinact;            /* inactive timeout (ms) (0:no timeout) */
    int tirecon;            /* reconnect interval (ms) (0:no reconnect) */
} tcpcli_t;
typedef struct {            /* ntrip control type */
    int state;              /* state (0:close,1:wait,2:connect) */
    int type;               /* type (0:server,1:client) */
    int nb;                 /* response buffer size */
    char url[256];          /* url for proxy */
    char mntpnt[256];       /* mountpoint */
    char user[256];         /* user */
    char passwd[256];       /* password */
    char str[NTRIP_MAXSTR]; /* mountpoint string for server */
    unsigned char buff[NTRIP_MAXRSP]; /* response buffer */
    tcpcli_t *tcp;          /* tcp client */
} ntrip_t;
static void strlock(stream_t *stream) { lock(&stream->lock); }
static void strunlock(stream_t *stream) { unlock(&stream->lock); }
/* get tick time ---------------------------------------------------------------
* get current tick in ms
* args   : none
* return : current tick in ms
*-----------------------------------------------------------------------------*/
extern unsigned int tickget(void)
{
#ifdef WIN32
	return (unsigned int)timeGetTime();
#else
	struct timespec tp = { 0 };
	struct timeval  tv = { 0 };

#ifdef CLOCK_MONOTONIC_RAW
	/* linux kernel > 2.6.28 */
	if (!clock_gettime(CLOCK_MONOTONIC_RAW, &tp)) {
		return tp.tv_sec * 1000u + tp.tv_nsec / 1000000u;
	}
	else {
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000u + tv.tv_usec / 1000u;
	}
#else
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000u + tv.tv_usec / 1000u;
#endif
#endif /* WIN32 */
}
/* decode tcp/ntrip path (path=[user[:passwd]@]addr[:port][/mntpnt[:str]]) ---*/
static void decodetcppath(const char *path, char *addr, char *port, char *user,
                          char *passwd, char *mntpnt, char *str)
{
    char buff[MAXSTRPATH],*p,*q;
    if (port) *port='\0';
    if (user) *user='\0';
    if (passwd) *passwd='\0';
    if (mntpnt) *mntpnt='\0';
    if (str) *str='\0';
    
    strcpy(buff,path);
    
    if (!(p=strrchr(buff,'@'))) p=buff;
    
    if ((p=strchr(p,'/'))) {
        if ((q=strchr(p+1,':'))) {
            *q='\0'; if (str) strcpy(str,q+1);
        }
        *p='\0'; if (mntpnt) strcpy(mntpnt,p+1);
    }
    if ((p=strrchr(buff,'@'))) {
        *p++='\0';
        if ((q=strchr(buff,':'))) {
             *q='\0'; if (passwd) strcpy(passwd,q+1);
        }
        if (user) strcpy(user,buff);
    }
    else p=buff;
    
    if ((q=strchr(p,':'))) {
        *q='\0'; if (port) strcpy(port,q+1);
    }
    if (addr) strcpy(addr,p);
}
static int errsock(void) {return errno;}
/* set socket option ---------------------------------------------------------*/
static int setsock(socket_t sock, char *msg)
{
    int bs=buffsize,mode=1;
#ifdef WIN32
    int tv=0;
#else
    struct timeval tv={0};
#endif
    
    if (setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(const char *)&tv,sizeof(tv))==-1||
        setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(const char *)&tv,sizeof(tv))==-1) {
        sprintf(msg,"sockopt error: notimeo");
        closesocket(sock);
        return 0;
    }
    if (setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char *)&bs,sizeof(bs))==-1||
        setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char *)&bs,sizeof(bs))==-1) {
        sprintf(msg,"sockopt error: bufsiz");
    }
    if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(const char *)&mode,sizeof(mode))==-1) {
        sprintf(msg,"sockopt error: nodelay");
    }
    return 1;
}
/* non-block connect ---------------------------------------------------------*/
static int connect_nb(socket_t sock, struct sockaddr *addr, socklen_t len)
{
#ifdef WIN32
    u_long mode=1; 
    int err;
    
    ioctlsocket(sock,FIONBIO,&mode);
    if (connect(sock,addr,len)==-1) {
        err=errsock();
        if (err==WSAEWOULDBLOCK||err==WSAEINPROGRESS||
            err==WSAEALREADY   ||err==WSAEINVAL) return 0;
        if (err!=WSAEISCONN) return -1;
    }
#else
    struct timeval tv={0};
    fd_set rs,ws;
    int err,flag;
    
    flag=fcntl(sock,F_GETFL,0);
    fcntl(sock,F_SETFL,flag|O_NONBLOCK);
    if (connect(sock,addr,len)==-1) {
        err=errsock();
        if (err!=EISCONN&&err!=EINPROGRESS&&err!=EALREADY) return -1;
        FD_ZERO(&rs); FD_SET(sock,&rs); ws=rs;
        if (select(sock+1,&rs,&ws,NULL,&tv)==0) return 0;
    }
#endif
    return 1;
}
/* non-block receive ---------------------------------------------------------*/
static int recv_nb(socket_t sock, unsigned char *buff, int n)
{
    struct timeval tv={0};
    fd_set rs;
    int ret,nr;
    
    FD_ZERO(&rs); FD_SET(sock,&rs);
    ret=select(sock+1,&rs,NULL,NULL,&tv);
    if (ret<=0) return ret;
    nr=recv(sock,(char *)buff,n,0);
    return nr<=0?-1:nr;
}
/* non-block send ------------------------------------------------------------*/
static int send_nb(socket_t sock, unsigned char *buff, int n)
{
    struct timeval tv={0};
    fd_set ws;
    int ret,ns;
    
    FD_ZERO(&ws); FD_SET(sock,&ws);
    ret=select(sock+1,NULL,&ws,NULL,&tv);
    if (ret<=0) return ret;
    ns=send(sock,(char *)buff,n,0);
    return ns<n?-1:ns;
}
/* generate tcp socket -------------------------------------------------------*/
static int gentcp(tcp_t *tcp, int type, char *msg)
{
    struct hostent *hp;
#ifdef SVR_REUSEADDR
    int opt=1;
#endif
    
    
    /* generate socket */
    if ((tcp->sock=socket(AF_INET,SOCK_STREAM,0))==(socket_t)-1) {
        sprintf(msg,"socket error (%d)",errsock());
        tcp->state=-1;
        return 0;
    }
    if (!setsock(tcp->sock,msg)) {
        tcp->state=-1;
        return 0;
    }
    memset(&tcp->addr,0,sizeof(tcp->addr));
    tcp->addr.sin_family=AF_INET;
    tcp->addr.sin_port=htons(tcp->port);
    
    if (type==0) { /* server socket */
    
#ifdef SVR_REUSEADDR
        /* multiple-use of server socket */
        setsockopt(tcp->sock,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt,
                   sizeof(opt));
#endif
        if (bind(tcp->sock,(struct sockaddr *)&tcp->addr,sizeof(tcp->addr))==-1) {
            sprintf(msg,"bind error (%d) : %d",errsock(),tcp->port);
            closesocket(tcp->sock);
            tcp->state=-1;
            return 0;
        }
        listen(tcp->sock,5);
    }
    else { /* client socket */
        if (!(hp=gethostbyname(tcp->saddr))) {
            sprintf(msg,"address error (%s)",tcp->saddr);
            closesocket(tcp->sock);
            tcp->state=0;
            tcp->tcon=ticonnect;
            tcp->tdis=tickget();
            return 0;
        }
        memcpy(&tcp->addr.sin_addr,hp->h_addr,hp->h_length);
    }
    tcp->state=1;
    tcp->tact=tickget();
    return 1;
}
/* disconnect tcp ------------------------------------------------------------*/
static void discontcp(tcp_t *tcp, int tcon){
    
    closesocket(tcp->sock);
    tcp->state=0;
    tcp->tcon=tcon;
    tcp->tdis=tickget();
}
/* connect server ------------------------------------------------------------*/
static int consock(tcpcli_t *tcpcli, char *msg)
{
    int stat,err;
    /* wait re-connect */
    if (tcpcli->svr.tcon<0||(tcpcli->svr.tcon>0&&
        (int)(tickget()-tcpcli->svr.tdis)<tcpcli->svr.tcon)) {
        return 0;
    }
    /* non-block connect */
    if ((stat=connect_nb(tcpcli->svr.sock,(struct sockaddr *)&tcpcli->svr.addr,
                         sizeof(tcpcli->svr.addr)))==-1) {
        err=errsock();
        sprintf(msg,"connect error (%d)",err);
        closesocket(tcpcli->svr.sock);
        tcpcli->svr.state=0;
        return 0;
    }
    if (!stat) { /* not connect */
        sprintf(msg,"connecting...");
        return 0;
    }
    sprintf(msg,"%s",tcpcli->svr.saddr);
    tcpcli->svr.state=2;
    tcpcli->svr.tact=tickget();
    return 1;
}
/* open tcp client -----------------------------------------------------------*/
static tcpcli_t *opentcpcli(const char *path, char *msg)
{
    tcpcli_t *tcpcli,tcpcli0={{0}};
    char port[256]="";
    if (!(tcpcli=(tcpcli_t *)malloc(sizeof(tcpcli_t)))) return NULL;
    *tcpcli=tcpcli0;
    decodetcppath(path,tcpcli->svr.saddr,port,NULL,NULL,NULL,NULL);
    if (sscanf(port,"%d",&tcpcli->svr.port)<1) {
        sprintf(msg,"port error: %s",port);
        free(tcpcli);
        return NULL;
    }
    tcpcli->svr.tcon=0;
    tcpcli->toinact=toinact;
    tcpcli->tirecon=ticonnect;
    return tcpcli;
}
/* close tcp client ----------------------------------------------------------*/
static void closetcpcli(tcpcli_t *tcpcli)
{  
    closesocket(tcpcli->svr.sock);
    free(tcpcli);
}
/* wait socket connect -------------------------------------------------------*/
static int waittcpcli(tcpcli_t *tcpcli, char *msg)
{
    
    if (tcpcli->svr.state<0) return 0;
    
    if (tcpcli->svr.state==0) { /* close */
        if (!gentcp(&tcpcli->svr,1,msg)) return 0;
    }
    if (tcpcli->svr.state==1) { /* wait */
        if (!consock(tcpcli,msg)) return 0;
    }
    if (tcpcli->svr.state==2) { /* connect */
        if (tcpcli->toinact>0&&
            (int)(tickget()-tcpcli->svr.tact)>tcpcli->toinact) {
            sprintf(msg,"timeout");
            discontcp(&tcpcli->svr,tcpcli->tirecon);
            return 0;
        }
    }
    return 1;
}
/* read tcp client -----------------------------------------------------------*/
static int readtcpcli(tcpcli_t *tcpcli, unsigned char *buff, int n, char *msg)
{
    int nr,err;
    if (!waittcpcli(tcpcli,msg)) return 0;
    
    if ((nr=recv_nb(tcpcli->svr.sock,buff,n))==-1) {
        if ((err=errsock())) {
            sprintf(msg,"recv error (%d)",err);
        }
        else {
            sprintf(msg,"disconnected");
        }
        discontcp(&tcpcli->svr,tcpcli->tirecon);
        return 0;
    }
    if (nr>0) tcpcli->svr.tact=tickget();
    return nr;
}
/* write tcp client ----------------------------------------------------------*/
static int writetcpcli(tcpcli_t *tcpcli, unsigned char *buff, int n, char *msg)
{
    int ns,err;
    if (!waittcpcli(tcpcli,msg)) return 0;
    
    if ((ns=send_nb(tcpcli->svr.sock,buff,n))==-1) {
        if ((err=errsock())) {
            sprintf(msg,"send error (%d)",err);
        }
        else {
            sprintf(msg,"disconnected");
        }
        discontcp(&tcpcli->svr,tcpcli->tirecon);
        return 0;
    }
    if (ns>0) tcpcli->svr.tact=tickget();
    return ns;
}
/* base64 encoder ------------------------------------------------------------*/
static int encbase64(char *str, const unsigned char *byte, int n)
{
    const char table[]=
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i,j,k,b;
    for (i=j=0;i/8<n;) {
        for (k=b=0;k<6;k++,i++) {
            b<<=1; if (i/8<n) b|=(byte[i/8]>>(7-i%8))&0x1;
        }
        str[j++]=table[b];
    }
    while (j&0x3) str[j++]='=';
    str[j]='\0';
    return j;
}
/* send ntrip server request -------------------------------------------------*/
static int reqntrip_s(ntrip_t *ntrip, char *msg)
{
    char buff[1024+NTRIP_MAXSTR],*p=buff;
    
    
    p+=sprintf(p,"SOURCE %s %s\r\n",ntrip->passwd,ntrip->mntpnt);
    p+=sprintf(p,"Source-Agent: NTRIP %s\r\n",NTRIP_AGENT);
    p+=sprintf(p,"STR: %s\r\n",ntrip->str);
    p+=sprintf(p,"\r\n");
    
    if (writetcpcli(ntrip->tcp,(unsigned char *)buff,p-buff,msg)!=p-buff) return 0;
    ntrip->state=1;
    return 1;
}
/* send ntrip client request -------------------------------------------------*/
static int reqntrip_c(ntrip_t *ntrip, char *msg)
{
    char buff[1024],user[512],*p=buff;
    
    p+=sprintf(p,"GET %s/%s HTTP/1.0\r\n",ntrip->url,ntrip->mntpnt);
    p+=sprintf(p,"User-Agent: NTRIP %s\r\n",NTRIP_AGENT);
    
    if (!*ntrip->user) {
        p+=sprintf(p,"Accept: */*\r\n");
        p+=sprintf(p,"Connection: close\r\n");
    }
    else {
        sprintf(user,"%s:%s",ntrip->user,ntrip->passwd);
        p+=sprintf(p,"Authorization: Basic ");
        p+=encbase64(p,(unsigned char *)user,strlen(user));
        p+=sprintf(p,"\r\n");
    }
    p+=sprintf(p,"\r\n");
    
    if (writetcpcli(ntrip->tcp,(unsigned char *)buff,p-buff,msg)!=p-buff) return 0;
    ntrip->state=1;
    return 1;
}
/* test ntrip server response ------------------------------------------------*/
static int rspntrip_s(ntrip_t *ntrip, char *msg)
{
    int i,nb;
    char *p,*q;
    ntrip->buff[ntrip->nb]='0';
    if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_OK_SVR))) { /* ok */
        q=(char *)ntrip->buff;
        p+=strlen(NTRIP_RSP_OK_SVR);
        ntrip->nb-=p-q;
        for (i=0;i<ntrip->nb;i++) *q++=*p++;
        ntrip->state=2;
        sprintf(msg,"%s/%s",ntrip->tcp->svr.saddr,ntrip->mntpnt);
        return 1;
    }
    else if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_ERROR))) { /* error */
        nb=ntrip->nb<MAXSTATMSG?ntrip->nb:MAXSTATMSG;
        strncpy(msg,(char *)ntrip->buff,nb); msg[nb]=0;
        if ((p=strchr(msg,'\r'))) *p='\0';
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    else if (ntrip->nb>=NTRIP_MAXRSP) { /* buffer overflow */
        sprintf(msg,"response overflow");
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    return 0;
}
/* test ntrip client response ------------------------------------------------*/
static int rspntrip_c(ntrip_t *ntrip, char *msg)
{
    int i;
    char *p,*q;
    ntrip->buff[ntrip->nb]='0';
    
    if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_OK_CLI))) { /* ok */
        q=(char *)ntrip->buff;
        p+=strlen(NTRIP_RSP_OK_CLI);
        ntrip->nb-=p-q;
        for (i=0;i<ntrip->nb;i++) *q++=*p++;
        ntrip->state=2;
        sprintf(msg,"%s/%s",ntrip->tcp->svr.saddr,ntrip->mntpnt);
        ntrip->tcp->tirecon=ticonnect;
        return 1;
    }
    if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_SRCTBL))) { /* source table */
        if (!*ntrip->mntpnt) { /* source table request */
            ntrip->state=2;
            sprintf(msg,"source table received");
            return 1;
        }
        sprintf(msg,"no mountp. reconnect...");
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        /* increase subsequent disconnect time to avoid too many reconnect requests */
        if (ntrip->tcp->tirecon>300000) ntrip->tcp->tirecon=ntrip->tcp->tirecon*5/4;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    else if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_HTTP))) { /* http response */
        if ((q=strchr(p,'\r'))) *q='\0'; else ntrip->buff[128]='\0';
        strcpy(msg,p);
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    else if (ntrip->nb>=NTRIP_MAXRSP) { /* buffer overflow */
        sprintf(msg,"response overflow");
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    return 0;
}
/* wait ntrip request/response -----------------------------------------------*/
static int waitntrip(ntrip_t *ntrip, char *msg)
{
    int n;
    char *p;
    
    if (ntrip->state<0) return 0; /* error */
    
    if (ntrip->tcp->svr.state<2) ntrip->state=0; /* tcp disconnected */
    
    if (ntrip->state==0) { /* send request */
        if (!(ntrip->type==0?reqntrip_s(ntrip,msg):reqntrip_c(ntrip,msg))) {
            return 0;
        }
    }
    if (ntrip->state==1) { /* read response */
        p=(char *)ntrip->buff+ntrip->nb;
        if ((n=readtcpcli(ntrip->tcp,(unsigned char *)p,NTRIP_MAXRSP-ntrip->nb-1,msg))==0) {
            return 0;
        }
        ntrip->nb+=n; ntrip->buff[ntrip->nb]='\0';
        /* wait response */
        return ntrip->type==0?rspntrip_s(ntrip,msg):rspntrip_c(ntrip,msg);
    }
    return 1;
}
/* open ntrip ----------------------------------------------------------------*/
static ntrip_t *openntrip(const char *path, int type, char *msg)
{
    ntrip_t *ntrip;
    int i;
    char addr[256]="",port[256]="",tpath[MAXSTRPATH];
    if (!(ntrip=(ntrip_t *)malloc(sizeof(ntrip_t)))) return NULL;
    ntrip->state=0;
    ntrip->type=type; /* 0:server,1:client */
    ntrip->nb=0;
    ntrip->url[0]='\0';
    ntrip->mntpnt[0]=ntrip->user[0]=ntrip->passwd[0]=ntrip->str[0]='\0';
    for (i=0;i<NTRIP_MAXRSP;i++) ntrip->buff[i]=0;
    
    /* decode tcp/ntrip path */
    decodetcppath(path,addr,port,ntrip->user,ntrip->passwd,ntrip->mntpnt,
                  ntrip->str);
    
    /* use default port if no port specified */
    if (!*port) {
        sprintf(port,"%d",type?NTRIP_CLI_PORT:NTRIP_SVR_PORT);
    }
    sprintf(tpath,"%s:%s",addr,port);
    
    /* ntrip access via proxy server */
    if (*proxyaddr) {
        sprintf(ntrip->url,"http://%s",tpath);
        strcpy(tpath,proxyaddr);
    }

	printf("%s", tpath);
    /* open tcp client stream */
    if (!(ntrip->tcp=opentcpcli(tpath,msg))) {
        free(ntrip);
        return NULL;
    }
    return ntrip;
}
/* read ntrip ----------------------------------------------------------------*/
static int readntrip(ntrip_t *ntrip, unsigned char *buff, int n, char *msg)
{
    int nb;
    if (!waitntrip(ntrip,msg)) return 0;
    
    if (ntrip->nb>0) { /* read response buffer first */
        nb=ntrip->nb<=n?ntrip->nb:n;
        memcpy(buff,ntrip->buff+ntrip->nb-nb,nb);
        ntrip->nb=0;
        return nb;
    }
    return readtcpcli(ntrip->tcp,buff,n,msg);
}

extern int strread(stream_t *stream,unsigned char *buff,int n){
    unsigned int tick;
    char *msg=stream->msg;
    int nr; 
    if (!(stream->mode&STR_MODE_R)||!stream->port) return 0;
    strlock(stream);
    switch (stream->type) {
        case STR_NTRIPCLI: nr=readntrip((ntrip_t *)stream->port,buff,n,msg); break;
        default:
            strunlock(stream);
            return 0;
    }
}
/* initialize stream -----------------------------------------------------------
* initialize stream struct
* args   : stream_t *stream IO  stream
* return : none
*-----------------------------------------------------------------------------*/
void strinit(stream_t *stream)
{
    stream->type=0;
    stream->mode=0;
    stream->state=0;
    stream->inb=stream->inr=stream->outb=stream->outr=0;
    stream->tick_i=stream->tick_o=stream->tact=stream->inbt=stream->outbt=0;
    initlock(&stream->lock);
    stream->port=NULL;
    stream->path[0]='\0';
    stream->msg [0]='\0';
}
int stropen(stream_t *stream, int type, int mode, const char *path)
{  
    stream->type=type;
    stream->mode=mode;
    strcpy(stream->path,path);
    stream->inb=stream->inr=stream->outb=stream->outr=0;
    stream->tick_i=stream->tick_o=tickget();
    stream->inbt=stream->outbt=0;
    stream->msg[0]='\0';
    stream->port=NULL;
    switch (type) {
        case STR_NTRIPCLI: stream->port=openntrip (path,1,   stream->msg); break;
        default: stream->state=0; return 1;
    }
    stream->state=!stream->port?-1:1;
    return stream->port!=NULL;
}
