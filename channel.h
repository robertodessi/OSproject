#ifndef CHANNEL_H
#define CHANNEL_H

#include <semaphore.h> //sem_t

#define MAX_LENGHT 50  //lunghezza massima del nome del canale

//struttura dati che rappresenta il canale
typedef struct {
    int dim; //dimensione dell'array client_desc
    int* client_desc; //array dei descrittori dei vari client connessi al canale
    int id; //identificatore del canale
    char* name_channel; //nome del canale
    int owner;//descrittore del client creatore del canale
} channel_struct;


//struttura dati che rappresenta la lista di canali
typedef struct {
    int num_channels; //dimensione dell'array channel
    char* channel[20]; //array dei canale
    int* fd;  //descrittore della pipe per comunicare con il canale  
} channel_list_struct;




//struttura dati che rappresenta i dati passati al thread che si occupa del canale
typedef struct handler_args_c {
    int fd;  //descrittore della pipe 
    channel_list_struct* channel_list;  //lista di tutti i canali
    channel_struct* my_channel;
} handler_args_c;


//funzione che esegue il thread del canale
void* channel_handler(void* arg);

//stampa le info di un canale
void printChannel(channel_struct* channel);

#endif /* CHANNEL_H */

