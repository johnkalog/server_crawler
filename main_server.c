#include "functions_server.h"

int main(int argc, char *argv[]){
  int serving_port,command_port,num_of_threads;
  argument_manage(argc,argv);
  serving_port = atoi(argv[2]);
  command_port = atoi(argv[4]);
  if ( serving_port==command_port ){
    printf("serving port and command port must be different\n");
    exit(1);
  }
  num_of_threads = atoi(argv[6]);
  struct pollfd fds[15];  //3 gia akribeia
  memset(fds,0,sizeof(fds));

  pthread_mutex_init(&mtx_server,NULL);
  pthread_cond_init(&cv_nonempty,NULL);

  list_info *buffer=malloc(sizeof(list_info));
  list_init(buffer);

  thread_arg *args=malloc(sizeof(thread_arg));
  args->shutdown = 0;
  args->buffer = buffer;
  args->dir = strdup(argv[8]);

  int i;
  pthread_t *thread_pool=malloc(num_of_threads*sizeof(pthread_t)),err;
  for ( i=0; i<num_of_threads; i++ ){
    if ( err=pthread_create(&thread_pool[i],NULL,thread_f,args) ){
      perror2 ("pthread_create",err);
      exit (1) ;
    }
  }

  struct sockaddr_in request_socket,command_socket;
  int request_fd,command_fd;

  //------------------------request socket socket bind listen------------------
  int request_size = sizeof(request_socket),r_fd;
  request_fd = socket_bind_listen(&request_socket,serving_port,request_size);
  //-----------------------end socket bind listen-----------------------------

  //------------------------command socket socket bind listen------------------
  int command_size=sizeof(command_socket),c_fd;
  command_fd = socket_bind_listen(&command_socket,command_port,command_size);
  //-----------------------end socket bind listen-----------------------------

  char buf[256];  //store here the command from telnet
  bzero(buf,256);

  fds[0].fd = request_fd; //0 for request ,1 for command,index 2 for telnet
  fds[0].events = POLLIN;
  fds[1].fd = command_fd;
  fds[1].events = POLLIN;

  int nfds=2,timeout=-1,result,push_fd; //nfds fd's exist in fds array
  int current_size=nfds,j,flag_command=0,flag=0;
  Data insert;

  pages=bytes=0;  //for STATS
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
        if ( fds[i].fd==request_fd ){
          if ( fds[i].revents==POLLIN ){  //an egine syndesh me thread tou crawler
            if ( (push_fd=accept(fds[i].fd,(struct sockaddr*)&request_socket,&request_size)) < 0){
              perror("accept");
              exit(1);
            }
            if ( err=pthread_mutex_lock(&mtx_server) ){
              perror2("pthread_mutex_lock",err);
              exit(1) ;
            }
            insert.fd = push_fd;  //push in buffer for procession from threads
            push_list(&insert,buffer,1);
            if ( err=pthread_mutex_unlock(&mtx_server) ){
              perror2("pthread_mutex_lock",err);
              exit(1) ;
            }
            pthread_cond_signal(&cv_nonempty);
          }
        }
        else if ( fds[i].fd==command_fd ){
          if ( fds[i].revents==POLLIN && flag_command==0 ){ //ekteleitai mia fora giati theloume na exoume mia sundesh telnet
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
              r = read(fds[i].fd,buf,256);
              buf[strlen(buf)-2] = '\0';  //logw telnet
              if ( strcmp(buf,"STATS")==0 ){
                float elapsed;
                struct timeval t1;
                gettimeofday(&t1,0);
                elapsed = timedifference_msec(t0, t1);
                 int seconds=elapsed/1000;
                 float milliseconds=elapsed-seconds*1000;
                 int minutes=seconds/60;
                 seconds=seconds-minutes*60;
                printf("Server up for %d:%d.%d, served %d pages, %d bytes\n",minutes,seconds,(int)milliseconds,pages,bytes);
              }
              else if ( strcmp(buf,"SHUTDOWN")==0 ){
                flag = 1;
                args->shutdown = 1;
                pthread_cond_broadcast(&cv_nonempty); //ksupnane ola gia na termatisoun
              }
              else{
                printf("Command %s is wrong\n",buf);
              }
              bzero(buf,256);
          }
        }
      }
    }
    current_size = nfds;
  }
  //-------------------------------------wait childs to end-----------------------------
  for ( i=0; i<num_of_threads; i++ ){
    if ( err=pthread_join(thread_pool[i],NULL)) { /* Wait for thread */
      perror2 ("pthread_join",err); /* termination */
      exit (1) ;
    }
  }
  //---------------------------------close fd's----------------------------------------
  for ( i=0; i<nfds; i++){
    if ( fds[i].fd>=0){
      close(fds[i].fd);
    }
  }

  if ( err=pthread_mutex_destroy(&mtx_server) ) {
    perror2("pthread_mutex_destroy",err);
    exit(1) ;
  }
  if ( err=pthread_cond_destroy(&cv_nonempty) ) {
    perror2("pthread_cond_destroy",err);
    exit(1) ;
  }
  free(thread_pool);
  free(args->dir);
  delete_list(buffer,1);
  free(args);

  return 0;
}
