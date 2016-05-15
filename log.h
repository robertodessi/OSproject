#ifndef LOG_H
#define LOG_H

//nome del file di log
#define NOME_FILE "log.txt"

#include <stdint.h>  //uint16_t

//scrive nel file di log la stringa toWrite
void logMsg(char* toWrite);

//resetta il file di log
void resetLog();

//log per le connessioni @param ip, port
void logConnection(char ip[], uint16_t port);

//log per gli errori @param err, err number
void logError(char* toWrite, int err);

//log per segFault error @param add is the address where the error occurred
void logSeg(unsigned int add);

#endif
