#include "functions_server.h"

float timedifference_msec(struct timeval t0,struct timeval t1){
    return (t1.tv_sec - t0.tv_sec)*1000.0f + (t1.tv_usec - t0.tv_usec)/1000.0f;
}

void send_200(int fd,char *file){ //file ths morfhs root_dir/site_.../page_....html
  int buf_size=10;
  char *daytime_f=make_daytime();
  FILE *fd_local=fopen(file,"r"); //pointer gia anoigma tou arxeiou
  if ( fd_local==NULL ){
    perror("fopen:");
    exit(1);
  }
  int length; //megeuow tou arxeiou
  fseek(fd_local,0,SEEK_END); //end of file
  length = ftell(fd_local); // get current file pointer
  fseek(fd_local,0,SEEK_SET); //beginning file again
  pages ++;
  bytes += length;
  char reply[1024];
  bzero(reply,1024);
  strcpy(reply,"HTTP/1.1 200 OK\r\n");
  strcat(reply,"Date: ");
  strcat(reply,daytime_f);
  strcat(reply,"\r\n");
  strcat(reply,"Server: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: ");
  char l_str[5];
  bzero(l_str,5);
  sprintf(l_str,"%d",length);
  strcat(reply,l_str);
  strcat(reply,"\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n");
  char *header=malloc((strlen(reply)+1)*sizeof(char));
  strcpy(header,reply);
  int h_len=strlen(header)+1;
  send(fd,header,h_len,MSG_NOSIGNAL);
  //write(fd,header,h_len);
  free(header);
  char *buf=malloc(buf_size*sizeof(char));
  bzero(buf,buf_size);
  while ( fgets(buf,buf_size,fd_local)!=NULL ){ //apostolh tou periexomenou tou .html ana buf_size bytes
    send(fd,buf,buf_size,MSG_NOSIGNAL);
    //write(fd,buf,buf_size);
    bzero(buf,buf_size);
    fflush(fd_local);
  }
  free(buf);
  fclose(fd_local);
}

void send_403(int fd){
  char *daytime_f=make_daytime();
  char reply[1024];
  bzero(reply,1024);
  strcpy(reply,"HTTP/1.1 403 Forbidden\r\n");
  strcat(reply,"Date: ");
  strcat(reply,daytime_f);
  strcat(reply,"\r\n");
  strcat(reply,"Server: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: ");
  char l_str[5];
  bzero(l_str,5);
  int length=strlen("<html>Trying to access this file but don't think I can make it.</html>");  //gia to periexomeno html
  sprintf(l_str,"%d",length);
  strcat(reply,l_str);
  strcat(reply,"\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n");
  char *header=malloc((strlen(reply)+1)*sizeof(char));
  strcpy(header,reply);
  int h_len=strlen(header)+1;
  send(fd,header,h_len,MSG_NOSIGNAL);
  free(header);
  //write(fd,header,h_len);
  char *to_send="<html>Trying to access this file but don't think I can make it.</html>";
  int len=strlen(to_send)+1;
  send(fd,to_send,len,MSG_NOSIGNAL);
  //write(fd,to_send,len);
}

void send_404(int fd){
  char *daytime_f=make_daytime();
  char reply[1024];
  bzero(reply,1024);
  strcpy(reply,"HTTP/1.1 404 Not Found\r\n");
  strcat(reply,"Date: ");
  strcat(reply,daytime_f);
  strcat(reply,"\r\n");
  strcat(reply,"Server: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: ");
  char l_str[5];
  bzero(l_str,5);
  int length=strlen("<html>Sorry dude, couldn't find this file.</html>");
  sprintf(l_str,"%d",length);
  strcat(reply,l_str);
  strcat(reply,"\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n");
  char *header=malloc((strlen(reply)+1)*sizeof(char));
  strcpy(header,reply);
  int h_len=strlen(header)+1;
  send(fd,header,h_len,MSG_NOSIGNAL);
  //write(fd,header,h_len);
  free(header);
  char *to_send="<html>Sorry dude, couldn't find this file.</html>";
  int len=strlen(to_send)+1;
  send(fd,to_send,len,MSG_NOSIGNAL);  //instead of write to avoid SIGPIPE
  //write(fd,to_send,len);
}

char *make_daytime(){ //dhmiourgia morfhs hmeromhnias
  time_t t=time(NULL);
  struct tm *tm = localtime(&t);
  char *asc=asctime(tm),*token,daytime[128];  //h asctime epistrefei char * ths morfhs Sat Mar 25 06:10:10 1989 gia auto ginetai katallhlh tropopoihsh
  bzero(daytime,128);
  token = strtok(asc," ");
  strcpy(daytime,token);
  strcat(daytime,", ");
  char *month=strtok(NULL," "),*date=strtok(NULL," "),*day_time=strtok(NULL," "),*year=strtok(NULL," \n");
  strcat(daytime,date);
  strcat(daytime," ");
  strcat(daytime,month);
  strcat(daytime," ");
  strcat(daytime,year);
  strcat(daytime," ");
  strcat(daytime,day_time);
  strcat(daytime," GMT");
  char *daytime_f=malloc((strlen(daytime)+1)*sizeof(char));
  strcpy(daytime_f,daytime);
  return daytime_f;
}

void manage_request(int fd,char *buf,char *dir){  //diaxeirhsh http request
  char *copy=strdup(buf),*line = strtok(copy,"\r\n"),*copy1=strdup(line),*copy2=strdup(line);
  char *token=strtok(copy1," ");
  int count=0;
  while ( token!=NULL ){
    count ++;
    token = strtok(NULL," ");
  }
  if ( count!=3 ){
    printf("Error GET request invalid\n");
    return;
  }
  token = strtok(copy2," ");
  if ( strcmp(token,"GET")!=0 ){
    printf("GET needed.Error\n");
    return;
  }
  token = strtok(NULL," ");
  char *site_path=strdup(token);
  token = strtok(NULL," ");
  if ( strcmp(token,"HTTP/1.1")!=0 ){
    printf("Error there is not HTTP/1.1\n");
    return;
  }
  if ( buf[strlen(buf)-4]!='\r' || buf[strlen(buf)-3]!='\n' || buf[strlen(buf)-2]!='\r' || buf[strlen(buf)-1]!='\n' ){  //h allagh grammhs epetai apo \r
    printf("Invalid http\n");
  }
  token = strtok(line," ");
  token = strtok(NULL," ");
  char *file=malloc((strlen(dir)+strlen(token)+1)*sizeof(char));
  strcpy(file,dir);
  strcat(file,token+1);
  file[strlen(file)] = '\0';
  if( access(file,F_OK)!=-1 ) {
    if( access(file,R_OK)!=-1 ) {
      send_200(fd,file);
    }
    else{
      send_403(fd);
    }
  }
  else{
    send_404(fd);
  }
  free(file);
  close(fd);
}

void* thread_f(void* argp) {
  thread_arg *my_args=argp;
  char buf[1024];
  bzero(buf,1024);
  int err,flag;
  Data pop_element;
  while ( my_args->shutdown==0 ){ //oso den exei dwthei SHUTDOWN
    flag = 0; //gia elegxo an se auto to loop egine pop apo ton buffer
    if ( err=pthread_mutex_lock(&mtx_server) ){
      perror2("pthread_mutex_lock",err);
      exit(1) ;
    }
    if ( my_args->buffer->length<=0 ){
      pthread_cond_wait(&cv_nonempty,&mtx_server);
    }
    if ( my_args->buffer->length>0 ){ //gia ta threads pou termatizoun
      pop(&pop_element,my_args->buffer,1);
      flag = 1;
      read(pop_element.fd,buf,1024);
    }
    if ( err=pthread_mutex_unlock(&mtx_server) ){
      perror2("pthread_mutex_lock",err);
      exit(1) ;
    }
    if ( flag==1 ){
      manage_request(pop_element.fd,buf,my_args->dir);
    }
  }
  pthread_exit(NULL) ;
}

void argument_manage(int argc,char *argv[]){
  if ( argc<9 ){
    printf("Less argumnets\n");
    exit(1);
  }
  if ( argc>9 ){
    printf("More argumnets\n");
    exit(1);
  }
  if ( strcmp(argv[1],"-p")!=0 ){
    printf("-p missed\n");
    exit(1);
  }
  if ( atoi(argv[2])==0 ){
    printf("Serving port must be number!=0\n");
    exit(1);
  }
  if ( strcmp(argv[3],"-c")!=0 ){
    printf("-c missed\n");
    exit(1);
  }
  if ( atoi(argv[4])==0 ){
    printf("Command port must be number!=0\n");
    exit(1);
  }
  if ( strcmp(argv[5],"-t")!=0 ){
    printf("-t missed\n");
    exit(1);
  }
  if ( atoi(argv[6])<1 ){
    printf("Num of threads must be number>=1\n");
    exit(1);
  }
  if ( strcmp(argv[7],"-d")!=0 ){
    printf("-d missed\n");
    exit(1);
  }
  struct stat sb;
  if ( !(stat(argv[8],&sb)==0 && S_ISDIR(sb.st_mode)) ){
    printf("Directory %s doesn't exist\n",argv[8]);
    exit(1);
  }
}
