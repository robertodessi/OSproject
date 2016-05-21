/* 
 * File:   thread_util.h
 * Author: Roberto
 *
 * Created on May 8, 2016, 12:25 PM
 */

#ifndef THREAD_UTIL_H
#define	THREAD_UTIL_H

#include "thread.h"
#include "common.h"

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()
#include <sys/types.h>
#include <sys/msg.h>

//ritorna il nome del canale prendendolo dal comando ricevuto lungo command_len contenuto in str
char* prendiNome(char* str, int len, size_t command_len);

//stampa le info di un canale
void printChannel(channel_struct* channel);

//stampa la lista di canali
void printList(channel_list_struct* list);

//invia a @dest (descrittore del client) la stringa @s
int invio(char* s, int dest);

//riceve da @dest (descrittore del client) e salva in @buf il messaggio - @buf_len indica la lunghezza del buffer
int ricevi(char* buf,size_t buf_len,int mitt,int id_coda,mymsg* recv_message,int* is_connect,sem_t* my_named_semaphore,channel_struct* my_channel);

//Legge il messaggio di tipo 1 in maniera NON bloccante e lo salva in @recv_message
//Ritorna 1 se c'è un messaggio, 0 se non c'è nessun messaggio ,-1 in caso di errore.
int leggiMSG(int id_coda, mymsg* recv_message);

//quando un canale sta per essere chiuso, questa funzione farà tutto il necessario per disconnettersi in maniera "pulita"
int esci(mymsg recv_message, int* is_connect,sem_t* my_named_semaphore,channel_struct* my_channel, int client_desc);


void freeChannel(channel_struct* channel);


#endif	/* THREAD_UTIL_H */
