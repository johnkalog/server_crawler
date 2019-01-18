#include "common.h"

pthread_mutex_t mtx_server ; /*mutex*/
pthread_cond_t cv_nonempty; /*the condition variable for non_empty buffer*/

typedef struct thread_arg{  //gia to orisma sta threads
  int shutdown;
  list_info *buffer;
  char *dir;
}thread_arg;

int pages,bytes;

float timedifference_msec(struct timeval,struct timeval);

void send_200(int,char *);

void send_403(int);

void send_404(int);

char *make_daytime();

void manage_request(int,char *,char *);

void* thread_f(void*);

void argument_manage(int argc,char *argv[]);
