#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>//warning strlen strcat
#include <errno.h>//per poter leggere il valore di errno
#include <time.h>//per giorno e data correnti
#include <semaphore.h>
#include <fcntl.h>  //O_CREATE
#include "log.h"

#define GENERIC_ERROR_HELPER(cond, errCode, msg)     do {           	\
        if (cond) {                                                 	\
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));	\
            logMsg(msg);						\
            exit(EXIT_FAILURE);                                     	\
        }                                                           	\
    } while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)

/* Configuration parameters */
#define DEBUG           1   // display debug messages
#define MAX_CONN_QUEUE  3   // max number of connections the server can queue
#define SERVER_ADDRESS  "127.0.0.1"
#define SHOW_COMMAND    "/show"
#define CREATE_COMMAND  "/create"
#define JOIN_COMMAND    "/join"
#define QUIT_COMMAND    "/quit"
#define DELETE_COMMAND  "/delete"
#define SERVER_PORT     2016
#define NAME_SEM 		"common_sem"
#define MINUTES 		10
#define MAX_CLIENT 		10
#define MAX_CHANNEL 	5
#define MAX_BYTES	150

//semaforo per channle_list_struct
sem_t* sem;

pthread_t threads[MAX_CLIENT];
int n_client;
int client_sock[MAX_CLIENT];


#endif

