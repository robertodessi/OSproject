#ifndef CHANNEL_H
#define CHANNEL_H


//struttura dati che rappresenta i dati passati al thread che si occupa del canale
typedef struct handler_args_s {
    int socket_desc;
    struct sockaddr_in* client_addr;
} handler_args_t;

//struttura dati che rappresenta il canale
typedef struct {
    int dim; //dimensione dell'array client_desc
    int* client_desc; //array dei descrittori dei vari client connessi al canale
    int id; //identificatore del canale
    int owner;//descrittore del client creatore del canale
} channel_struct;



//funzione che esegue il thread del canale
void* welcome_handler(void* arg);

//stampa le info di un canale
void printChannel(channel_struct* channel);

#endif /* CHANNEL_H */

