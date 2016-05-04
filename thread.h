#ifndef THREAD_H
#define THREAD_H

#include "common.h" 

#define FLAG    0
#define SIZE  125


//struttura dati che rappresenta il canale
typedef struct {
    int dim; //dimensione dell'array client_desc
    int* client_desc; //array dei descrittori dei vari client connessi al canale
    int id;  //identificatore del canale
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
} handler_args_t;


//struttura dati per la coda di messaggi
typedef struct {
    long mtype;
    char mtext[SIZE];
} mymsg;


//funzione che esegue il thread del client
void* connection_handler(void* arg);

//ritorna il nome del canale prendendolo dal comando ricevuto lungo command_len contenuto in str
char* prendiNome(char* str, int len, size_t command_len);

//stampa le info di un canale
void printChannel(channel_struct* channel);

//stampa la lista di canali
void printList(channel_list_struct* list);

//invia a @dest (descrittore del client) la stringa @s
void invio(char* s, int dest);

//riceve da @dest (descrittore del client) e salva in @buf il messaggio - @buf_len indica la lunghezza del buffer
int ricevi(char* buf, size_t buf_len, int mitt);

//legge il messaggio di tipo 1 in maniera NON bloccante. Ritorna 1 se c'è un messaggio, 0 se non c'è nessun messaggio ,-1 in caso di errore.
int leggiMSG();

//quando un canale sta per essere chiuso, questa funzione farà tutto il necessario per disconnettersi in maniera "pulita"
void esci();

#endif 
