#include "functions_crawler.h"

float timedifference_msec(struct timeval t0,struct timeval t1){
    return (t1.tv_sec - t0.tv_sec)*1000.0f + (t1.tv_usec - t0.tv_usec)/1000.0f;
}

void make_link_push(char *aft,list_info *buffer,list_info *history,char *ip_port){  //dhmiourgia link kai eisagwgh ston buffer
  int err;
  char *after_dir=strdup(strstr(aft,"/site"));
  char *link=malloc((strlen(ip_port)+strlen(after_dir)+1)*sizeof(char));
  strcpy(link,ip_port);
  strcat(link,after_dir);
  free(after_dir);
  if ( is_link_here(link,history)==0 ){ //an den exei zhththei prin
    if ( err=pthread_mutex_lock(&mtx_crawler) ){
      perror2("pthread_mutex_lock",err);
      exit(1) ;
    }
    if ( is_link_here(link,buffer)==0 ){  //an den einai twra ston buffer
      Data for_insert;
      for_insert.str = strdup(link);
      push_list(&for_insert,buffer,2);
      pthread_cond_signal(&cv_nonempty);
      free(for_insert.str);
    }
    if ( err=pthread_mutex_unlock(&mtx_crawler) ){
      perror2("pthread_mutex_unlock",err);
      exit(1) ;
    }
  }
  free(link);
}

void manage_reply(int fd,char *full_path,char *save_dir,list_info *buffer,list_info *history,char *ip_port){  //diaxeirish response
  char *path = strdup(strstr(full_path,"/site")); //apothhkeush tou /site_.../page...thml
  int buf_size=10;
  char buf[1024],buf_html[1024],*token;
  int header_l,html_l;
  bzero(buf,1024);
  bzero(buf_html,1024);
  read(fd,buf,1024);
  header_l = strlen(buf);
  char *copy=malloc((strlen(buf)+1)*sizeof(char));
  strcpy(copy,buf);
  char *copy_for_length=strdup(copy);
  char *for_length_token,*for_length_line;
  token = strtok(copy," ");
  token = strtok(NULL," ");
  if ( strcmp(token,"200")==0 ){
    printf("%s\n",buf);
    for_length_line=strtok(copy_for_length,"\r\n"); //ypologismos tou megethous apo to header
    for_length_line=strtok(NULL,"\r\n");
    for_length_line=strtok(NULL,"\r\n");
    for_length_line=strtok(NULL,"\r\n");
    for_length_token=strtok(for_length_line," ");
    for_length_token=strtok(NULL,"\r\n");
    pages ++;
    bytes += atoi(for_length_token);
    char *copy=strdup(path),*site=strtok(copy,"/");
    char *full_site=malloc((strlen(save_dir)+strlen(site)+2)*sizeof(char)); //full site to path gia to site gia dhmiourgia dir mesa ston save_dir an den uparxei
    strcpy(full_site,save_dir);
    strcat(full_site,"/");
    strcat(full_site,site);
    char *full_path=malloc((strlen(save_dir)+strlen(path)+1)*sizeof(char));
    strcpy(full_path,save_dir);
    strcat(full_path,path);
    struct stat sb;
    if ( !((stat(full_site,&sb)==0 && S_ISDIR(sb.st_mode))) ){ //if save_dir/site_x does not exist create it's directory
      struct stat st = {0};
      if ( stat(full_site,&st)==-1 ) {
        mkdir(full_site, 0700);
      }
    }
    FILE *fp=fopen(full_path,"w");
    char *buffer_r=malloc(buf_size*sizeof(char));
    bzero(buffer_r,buf_size);
    int num=read(fd,buffer_r,buf_size);
    while ( num>0 ){
      fputs(buffer_r,fp);
      bzero(buffer_r,buf_size);
      num=read(fd,buffer_r,buf_size);
      if ( num<0 ){
        //break;
      }
    }
    fclose(fp);
    fflush(stdout);
    fp = fopen(full_path,"r");
    char *line=NULL,*after_dir;
    size_t n;
    while ( getline(&line,&n,fp)!=EOF ){  //anagnwsh kathe grammhs kai apothhkeush twn link
      if ( strstr(line,"<a href=")!=NULL ){
        after_dir = strtok(line,">");
        after_dir = strtok(NULL,">");
        char *aft=strtok(after_dir,"<");
        make_link_push(aft,buffer,history,ip_port);
      }
    }
    fclose(fp);
    free(full_site);
    free(full_path);
    free(buffer_r);
    free(path);
  }
  else if ( strcmp(token,"403")==0 ){
    read(fd,buf_html,1024);
    html_l = strlen(buf_html);
    int all_l=header_l+html_l+1;
    char *all=malloc(all_l*sizeof(char));
    bzero(all,all_l);
    strncpy(all,buf,header_l);
    strcat(all,buf_html);
    printf("%s\n",all);
    free(all);
  }
  else if ( strcmp(token,"404")==0 ){
    read(fd,buf_html,1024);
    html_l = strlen(buf_html);
    int all_l=header_l+html_l+1;
    char *all=malloc(all_l*sizeof(char));
    bzero(all,all_l);
    strncpy(all,buf,header_l);
    strcat(all,buf_html);
    printf("%s\n",all);
    free(all);
  }
}

void make_request(char *full_path,char *http,char *ip_port,char *arg2){ //dhmiourgia request
  char *path = strdup(strstr(full_path,"/site"));
  char *user="User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n";
  char *accept="\r\nAccept-Language: en-us\r\nAccept-Encoding: gzip, deflate\r\nConnection: Keep-Alive\r\n\r\n";
  strcpy(http,"GET ");
  strcat(http,path);
  strcat(http," HTTP/1.1\r\n");
  strcat(http,user);
  strcat(http,"Host: ");
  strcat(http,arg2);
  strcat(http,accept);
  free(path);
}

void* thread_f(void* argp) {
  thread_arg *my_args=argp;
  struct sockaddr_in client;
  Data for_get;
  for_get.str = NULL;
  int client_size=sizeof(client),socket_client_fd,flag,err;
  char *full_path;
  while ( my_args->shutdown==0 ){
    flag = 0;
    if ( err=pthread_mutex_lock(&mtx_crawler) ){
      perror2("pthread_mutex_lock",err);
      exit(1) ;
    }
    if ( my_args->buffer->length<=0 ){
      pthread_cond_wait(&cv_nonempty,&mtx_crawler);
    }
    if ( my_args->buffer->length>0 ){
      socket_client_fd = socket_connect(&client,my_args->port,client_size,my_args->arg2);
      if ( for_get.str!=NULL ){
        free(for_get.str);
      }
      pop(&for_get,my_args->buffer,2);
      push_list(&for_get,my_args->history,2);
      flag = 1;

      full_path = strdup(for_get.str);
      char http[1024];
      bzero(http,1024);
      make_request(full_path,http,my_args->ip_port,my_args->arg2);
      int send_size=strlen(http) + 1;
      send(socket_client_fd,http,send_size,MSG_NOSIGNAL); //avoid of SIGPIPE
      //write(socket_client_fd,http,1024);
    }
    if ( err=pthread_mutex_unlock(&mtx_crawler) ){
      perror2("pthread_mutex_lock",err);
      exit(1) ;
    }
    if ( flag==1 ){
      manage_reply(socket_client_fd,full_path,my_args->save_dir,my_args->buffer,my_args->history,my_args->ip_port);
      //close(socket_client_fd);  //prin na teleiwsei o server
    }
  }
  pthread_exit(NULL) ;
}

int socket_connect(struct sockaddr_in *the_socket,int port,int socket_size,char *arg2){ //sundesh me socket apo ton server
  int socket_fd;
  if ( (socket_fd=socket(AF_INET,SOCK_STREAM,0))<0 ){
    perror("socket");
  }
  char first_letter=arg2[0];
  the_socket->sin_family = AF_INET;
  the_socket->sin_port = htons(port);

  //am exei dwthei ip h hostname
  if ( first_letter-'0'>0 && first_letter-'0'<=9 ){ //ip
    the_socket->sin_addr.s_addr = inet_addr(arg2);
  }
  else{ //host_name
  struct hostent *rem=gethostbyname(arg2);
    if ( rem==NULL ){
      herror("gethostbyname");
      exit(1) ;
    }
    memcpy(&the_socket->sin_addr,rem->h_addr,rem->h_length);
  }
  if ( connect(socket_fd,(struct sockaddr*)the_socket,socket_size)<0 ){
    perror("connect");
  }
  return socket_fd;
}

void argument_manage(int argc,char *argv[]){
  if ( argc<12 ){
    printf("Less argumnets\n");
    exit(1);
  }
  if ( argc>12 ){
    printf("More argumnets\n");
    exit(1);
  }
  if ( strcmp(argv[1],"-h")!=0 ){
    printf("-h missed\n");
    exit(1);
  }
  if ( strcmp(argv[3],"-p")!=0 ){
    printf("-p missed\n");
    exit(1);
  }
  if ( atoi(argv[4])==0 ){
    printf("port must be number!=0\n");
    exit(1);
  }
  if ( strcmp(argv[5],"-c")!=0 ){
    printf("-c missed\n");
    exit(1);
  }
  if ( atoi(argv[6])==0 ){
    printf("Command port must be number!=0\n");
    exit(1);
  }
  if ( strcmp(argv[7],"-t")!=0 ){
    printf("-t missed\n");
    exit(1);
  }
  if ( atoi(argv[8])<1 ){
    printf("Num of threads must be number>=1\n");
    exit(1);
  }
  if ( strcmp(argv[9],"-d")!=0 ){
    printf("-d missed\n");
    exit(1);
  }
  struct stat sb;
  if ( (stat(argv[10],&sb)==0 && S_ISDIR(sb.st_mode)) ){
    printf("Directory %s already exists.Delete it first\n",argv[10]);
    exit(1);
  }
}
