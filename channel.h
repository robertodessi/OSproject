#ifndef CHANNEL_H
#define CHANNEL_H

#include <semaphore.h> //sem_t



//struttura dati che rappresenta il canale
typedef struct {
    int dim; //dimensione dell'array client_desc
    int* client_desc; //array dei descrittori dei vari client connessi al canale
    int id; //identificatore del canale
    int owner;//descrittore del client creatore del canale
} channel_struct;

//struttura dati che rappresenta la lista di canali
typedef struct {
    int num_channels; //dimensione dell'array channel
    channel_struct* channel; //array dei canale
} channel_list_struct;

//struttura dati che rappresenta i dati passati al thread che si occupa del canale
typedef struct handler_args_s {
    int socket_desc; //descrittore client
    struct sockaddr_in* client_addr; //address del client
    channel_list_struct* channel_list;  //lista dei canali
} handler_args_t;



//funzione che esegue il thread del canale
void* connection_handler(void* arg);

//stampa le info di un canale
void printChannel(channel_struct* channel);

#endif /* CHANNEL_H */

