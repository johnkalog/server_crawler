#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define perror2(s,e) fprintf(stderr,"%s:%s\n",s,strerror(e))

typedef union Data{
  int fd;
  char *str;
} Data;

typedef struct list{
  Data data;
  struct list *next;
}list;

typedef struct list_info{
  int length;
  list *first;
}list_info;

int socket_bind_listen(struct sockaddr_in *,int,int);

void delete_list(list_info *,int);

int is_link_here(char *,list_info *);

void pop(Data *,list_info *,int);

void push_list(Data *,list_info *,int);

void list_init(list_info *);
