#ifndef THREAD_H
#define THREAD_H

#include "common.h" 

#define FLAG    0
#define SIZE  125


//struttura dati che rappresenta il canale
typedef struct {
    int dim; //dimensione dell'array client_desc
    int* client_desc; //array dei descrittori dei vari client connessi al canale
    int* id;  //id dei thread
    char* name_channel; //nome del canale
    int owner;  //descrittore del client creatore del canale
} channel_struct;


//struttura dati che rappresenta la lista di canali
typedef struct {
    int num_channels; 	//dimensione dell'array channel
    char** name_channel; 	//array dei nomi canale
    channel_struct** channel; 	//array di puntatori alla struttura dati canale  
} channel_list_struct;



//struttura dati che rappresenta i dati passati al thread che si occupa del client
typedef struct handler_args_s {
    int socket_desc; //descrittore client
    struct sockaddr_in* client_addr; //address del client
    channel_list_struct* channel_list;  //lista dei canali
    int server_sd;  //server queue id
} handler_args_t;


//struttura dati per la coda di messaggi
typedef struct {
    long mtype;
    char mtext[SIZE];
} mymsg;


//funzione che esegue il thread del client
void* connection_handler(void* arg);



#endif 
