#ifndef CHANNEL_H
#define CHANNEL_H


//struttura dati che rappresenta i dati passati al thread che si occupa del canale
typedef struct handler_args_s {
    int socket_desc;
    struct sockaddr_in* client_addr;
} handler_args_t;


//funzione che esegue il thread del canale
void* connection_handler(void* arg);




#endif /* CHANNEL_H */

