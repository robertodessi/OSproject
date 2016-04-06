#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>//warning strlen strcat
#include <errno.h>//per poter leggere il valore di errno
#include <time.h>//per giorno e data correnti
#include <semaphore.h>
#include "log.h"

#define GENERIC_ERROR_HELPER(cond, errCode, msg)     do {           	\
        if (cond) {                                                 	\
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));		\
            logMsg(msg);												\
            exit(EXIT_FAILURE);                                     	\
        }                                                           	\
    } while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)

/* Configuration parameters */
#define DEBUG           1   // display debug messages
#define MAX_CONN_QUEUE  3   // max number of connections the server can queue
#define SERVER_ADDRESS  "127.0.0.1"
#define CREATE_COMMAND  "CREATE"
#define JOIN_COMMAND  "JOIN"
#define QUIT_COMMAND  "QUIT"
#define SERVER_PORT     2016


//semaforo per channle_list_struct
sem_t* sem;

#endif

