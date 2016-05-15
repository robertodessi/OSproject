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

void logChannel(char* command, char* channel, char ip[]);

/*log quando il server riceve un messaggio 
 *
 *  @param recv_bytes : bytes received from client 
 *  @param buf        : buffer containing message
 *  @param ip         : ip of the sender
 *  @param crash      : flag showing if client closed the conncection
 */ 
void logRec(int recv_bytes, char* buf, char ip[], int crash);


 /* log method for client actions, name channel is the name of the channel linked to the thread, ip is the client ip,
  * and flag shows which of the following actions must be written to the log file:
  * 0 quit
  * 1 delete
  * 2 fine vita del thread
  */
void logExit(int flags,char* name_channel, char ip[]);

#endif
