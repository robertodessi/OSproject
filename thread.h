#ifndef THREAD_H
#define THREAD_H

#include "channel.h" 

//struttura dati che rappresenta i dati passati al thread che si occupa del client
typedef struct handler_args_s {
    int socket_desc; //descrittore client
    struct sockaddr_in* client_addr; //address del client
    channel_list_struct* channel_list;  //lista dei canali
} handler_args_t;


//funzione che esegue il thread del client
void* connection_handler(void* arg);

//ritorna il nome del canale prendendolo dal comando ricevuto contenuto in buf
char* prendiNome(char* str);

#endif 
