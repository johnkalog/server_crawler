#include "functions_crawler.h"

int main(int argc, char *argv[]){
  int port,command_port,num_of_threads;
  char *save_dir,*ip_port,*starting_URL;
  argument_manage(argc,argv);
  port = atoi(argv[4]);
  command_port = atoi(argv[6]);
  num_of_threads = atoi(argv[8]);
  if ( port==command_port ){
    printf("port and command port must be different\n");
    exit(1);
  }

  starting_URL = strdup(argv[11]);

  int len=strlen(argv[2])+strlen(argv[4])+9;
  ip_port = malloc(len*sizeof(char)); //ip_port xwris / sto telos px http://linux09.di.uoa.gr:8081
  strcpy(ip_port,"http://");
  strcat(ip_port,argv[2]);
  strcat(ip_port,":");
  strcat(ip_port,argv[4]);
  ip_port[len-1] = '\0';

  save_dir = strdup(argv[10]);

  struct stat st = {0};
  if ( stat(save_dir,&st)==-1 ) {
    mkdir(save_dir, 0700);
  }

  pthread_mutex_init(&mtx_crawler,NULL);
  pthread_cond_init(&cv_nonempty,NULL);

  list_info *buffer=malloc(sizeof(list_info));
  list_init(buffer);

  list_info *history=malloc(sizeof(list_info)); //giat to istoriko twn arxeiwn pou exoun zhththeui apo ton server
  list_init(history);

  thread_arg *args=malloc(sizeof(thread_arg));
  args->buffer = buffer;
  args->history = history;
  args->arg2 = strdup(argv[2]);
  args->port = port;
  args->ip_port = strdup(ip_port);
  args->save_dir = strdup(save_dir);
  args->shutdown = 0;

  Data for_insert;
  for_insert.str = strdup(starting_URL);
  push_list(&for_insert,buffer,2);
  pthread_cond_signal(&cv_nonempty);
  free(for_insert.str);

  int i;
  pthread_t *thread_pool=malloc(num_of_threads*sizeof(pthread_t)),err;
  for ( i=0; i<num_of_threads; i++ ){
    if ( err=pthread_create(&thread_pool[i],NULL,thread_f,args) ){
      perror2 ("pthread_create",err);
      exit (1) ;
    }
  }

  struct sockaddr_in command_socket;
  int command_fd;

  //------------------------command socket socket bind listen------------------
  int command_size=sizeof(command_socket),c_fd;
  command_fd = socket_bind_listen(&command_socket,command_port,command_size);
  //-----------------------end socket bind listen-----------------------------

  struct pollfd fds[1]; //gia to command
  fds[0].fd = command_fd;
  fds[0].events = POLLIN;

  int nfds=1,result,timeout=-1,flag_command=0,flag=0,current_size=nfds;

  char buf[256];
  bzero(buf,256);

  pages=bytes=0;
  struct timeval t0;
  gettimeofday(&t0,0);

  while ( flag==0 ){
    result = poll(fds,nfds,timeout);
    if ( result<0 ){
      perror("  poll() failed");
      break;
    }
    else if ( result==0 ){
      printf("poll() timed out.End program.\n");
      break;
    }
    else if ( result>=1 ){
      for ( i=0; i<current_size; i++ ){
        if ( fds[i].fd==command_fd ){
          if ( fds[i].revents==POLLIN && flag_command==0 ){
            if ( (fds[nfds].fd=accept(command_fd,(struct sockaddr*)&command_socket,&command_size)) < 0){
              perror("accept");
              exit(1);
            }
            fds[nfds].events = POLLIN;
            nfds ++;
            flag_command = 1;
          }
        }
        else{
          if ( fds[i].revents==POLLIN ){
              int r;
              r = read(fds[i].fd,buf,256); //h epanw
              buf[strlen(buf)-2] = '\0';
              if ( strcmp(buf,"STATS")==0 ){
                float elapsed;
                struct timeval t1;
                gettimeofday(&t1,0);
                elapsed = timedifference_msec(t0, t1);
                 int seconds=elapsed/1000;
                 float milliseconds=elapsed-seconds*1000;
                 int minutes=seconds/60;
                 seconds=seconds-minutes*60;
                printf("Crawler up for %d:%d.%d, downloaded %d pages, %d bytes\n",minutes,seconds,(int)milliseconds,pages,bytes);
              }
              else if ( strcmp(buf,"SHUTDOWN")==0 ){
                flag = 1;
                args->shutdown = 1;
                pthread_cond_broadcast(&cv_nonempty);
              }
              else{
                char *copy_command=malloc((strlen(buf)+1)*sizeof(char));
                bzero(copy_command,strlen(buf)+1);
                strcpy(copy_command,buf);
                char *copy_c=strdup(copy_command);
                char *first_word=strtok(copy_command," ");
                if ( strcmp(first_word,"SEARCH")==0 ){
                  if ( buffer->length==0 ){ //prepei na exei teleiwsi to crawling giati mporei o buffer na adeiasei kai na ksanampei stoixeio endiamesa tou crawling
                    //printf("okkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk\n");
                  }
                  else{
                    printf("crawling still in-process\n");
                  }
                }
                else{
                  printf("Command %s is wrong\n",buf);
                }
              }
              bzero(buf,256);
          }
        }
      }
    }
    current_size = nfds;
  }
  //--------------------------------------------------------wait childs end---------------------------------------------------------
  for ( i=0; i<num_of_threads; i++ ){
    if ( err=pthread_join(thread_pool[i],NULL)) { /* Wait for thread */
      perror2 ("pthread_join",err); /* termination */
    }
  }
  for ( i=0; i<nfds; i++){
    if ( fds[i].fd>=0){
      close(fds[i].fd);
    }
  }
  if ( err=pthread_mutex_destroy(&mtx_crawler) ) {
    perror2("pthread_mutex_destroy",err);
    exit(1) ;
  }
  if ( err=pthread_cond_destroy(&cv_nonempty) ) {
    perror2("pthread_cond_destroy",err);
    exit(1) ;
  }

  free(thread_pool);
  free(args->arg2);
  free(args->ip_port);
  free(args->save_dir);
  free(starting_URL);
  delete_list(buffer,2);
  //delete_list(history,2);
  free(args);

  return 0;
}
