#include "common.h"

pthread_mutex_t mtx_crawler ; /*mutex*/
pthread_cond_t cv_nonempty;

typedef struct thread_arg{
  int shutdown;
  list_info *buffer,*history;
  char *arg2,*ip_port,*save_dir;
  int port;

}thread_arg;

int pages,bytes;

float timedifference_msec(struct timeval,struct timeval);

void make_link_push(char *,list_info *,list_info *,char *);

void manage_reply(int,char *,char *,list_info *,list_info *,char *);

void make_request(char *,char *,char *,char *);

void* thread_f(void*);

int socket_connect(struct sockaddr_in *,int,int,char *);

void argument_manage(int argc,char *argv[]);
