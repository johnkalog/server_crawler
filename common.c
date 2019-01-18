#include "common.h"

int socket_bind_listen(struct sockaddr_in *the_socket,int port,int socket_size){
  int socket_fd;
  if ( (socket_fd=socket(AF_INET,SOCK_STREAM,0))<0 ){
    perror("socket");
    exit(1);
  }
  the_socket->sin_family = AF_INET;
  the_socket->sin_addr.s_addr = htonl(INADDR_ANY);
  the_socket->sin_port = htons(port);
  if ( bind(socket_fd,(struct sockaddr*)the_socket,socket_size)<0 ){
    perror("bind");
    exit(1);
  }
  if ( listen(socket_fd,5)<0 ){
    perror("listen");
    exit(1);
  }
  return socket_fd;
}

void delete_list(list_info *buffer,int int_or_char){
  if ( buffer->first==NULL ){
    return;
  }
  list *tmp=buffer->first,*del;
  while ( tmp!=NULL ){
    del = tmp;
    buffer->first = tmp->next;
    free(del);
    if ( int_or_char==2 ){
      free(del->data.str);
    }
    del = NULL;
    tmp = tmp->next;
  }
  free(buffer);
  buffer = NULL;
}

int is_link_here(char *link,list_info *buffer){ //only for strings returns 1 if true
  if ( buffer->first==NULL ){
    return 0;
  }
  list *tmp=buffer->first;
  while ( tmp!=NULL ){
    if ( strcmp(tmp->data.str,link)==0 ){
      return 1;
    }
    tmp = tmp->next;
  }
  return 0;
}

void pop(Data *to_store,list_info *buffer,int int_or_char){ //analogo pop apo ton buffer logw tou int_or_char
  if ( buffer->first==NULL ){
    return;
  }
  if ( int_or_char==1 ){
    to_store->fd = buffer->first->data.fd;
  }
  else if ( int_or_char==2 ){
    to_store->str = strdup(buffer->first->data.str);
  }
  list *tmp;
  tmp = buffer->first;
  buffer->first = tmp->next;
  buffer->length --;
  if ( int_or_char==2 ){
    free(tmp->data.str);
  }
  free(tmp);
}

void push_list(Data *for_insert,list_info *buffer,int int_or_char){ //insert at end 1 for int 2 for char*
  list *current=buffer->first;
  if ( current==NULL ){
    buffer->first = malloc(sizeof(list));
    if ( int_or_char==1 ){
      buffer->first->data.fd = for_insert->fd;
    }
    else if ( int_or_char==2 ){
      buffer->first->data.str = strdup(for_insert->str);
    }
    buffer->first->next = NULL;
    buffer-> length ++;
    return;
  }
  while ( current->next!=NULL ){
    current = current->next;
  }
  current->next = malloc(sizeof(list));
  if ( int_or_char==1 ){
    current->next->data.fd = for_insert->fd;
  }
  else if ( int_or_char==2 ){
    current->next->data.str = strdup(for_insert->str);
  }
  current->next->next = NULL;
  buffer->length ++;
}

void list_init(list_info *buffer){  //arxikopoihsh listas
  buffer->length = 0;
  buffer->first = NULL;
}
