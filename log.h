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

/*log quando il server riceve un messaggio 
 *
 *  @param recv_bytes : bytes received from client 
 *  @param buf        : buffer containing message
 *  @param ip         : ip of the sender
 *  @param crash      : flag showing if client closed the conncection
 */ 
void logRec(int recv_bytes, char* buf, char ip[], int crash);

#endif
