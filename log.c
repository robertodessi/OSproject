//====================================
//			LOG WRITER
//====================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> //write()
#include <string.h>//warning strlen strcat
#include <fcntl.h>// serve per le opzioni di apertura del file

#include "common.h"
#include "log.h"


//  file descriptor for server log
FILE* file;

void logMsg(char* toWrite) {

    int ret;

    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;
    // ret = (file == null) ? -1 : 0;
    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);

    ret = fprintf(file, "%s%s\n\n\n", date, toWrite);
    ERROR_HELPER(ret, "Errore scrittura operazione server su file");

    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");

}

void logError(char* toWrite, int err) {

    int ret;

    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;

    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);

    ret = fprintf(file, "%s%s | Error number: %d\n\n\n", date, toWrite, err);
    ERROR_HELPER(ret, "Errore scrittura operazione server su file");

    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");

}

void logSeg(unsigned int add) {

    int ret;

    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;

    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);

    ret = fprintf(file, "%s | Segfault occurred (address is %x)", date, add);
    ERROR_HELPER(ret, "Errore scrittura operazione server su file");

    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");

}

void logConnection(char ip[], uint16_t port) {
    int ret;

    unsigned int client_port = (unsigned int) port;

    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;

    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);

    ret = fprintf(file, "%sIncoming connection accepted from IP: %s | PORT: %d\n\n\n", date, ip, client_port);
    ERROR_HELPER(ret, "Errore scrittura operazione server su file");

    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");
}

void logRec(int recv_bytes, char* buf, char ip[],  int crash) {

    int ret;

    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;
    
    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);
    
    if(!crash){
        ret = fprintf(file, "%sMessage received from client with IP %s. %d bytes received. Message says: %s\n\n\n", date, ip, recv_bytes, buf);
        ERROR_HELPER(ret, "Errore scrittura operazione server su file");
    }else{
        ret = fprintf(file, "%sClient with IP seems disconneted: %s thread is exiting.\n\n\n", date, ip);
        ERROR_HELPER(ret, "Errore scrittura operazione server su file");
    }
    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");
}

void logChannel(char* command, char* channel, char ip[]){
    int ret;

    //CREATE
    char* create_command = CREATE_COMMAND;
    size_t create_command_len = strlen(create_command);
    
    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;
    // ret = (file == null) ? -1 : 0;
    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);

    if(!memcmp(command, create_command, create_command_len)){
        ret = fprintf(file, "%sClient with IP %s just created a channel named: %s\n\n\n", date, ip, channel);
        ERROR_HELPER(ret, "Errore scrittura operazione server su file");
    }else {
        ret = fprintf(file, "%sClient with IP %s just joined channel named: %s\n\n\n", date, ip, channel);
        ERROR_HELPER(ret, "Errore scrittura operazione server su file");
    }
    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");
    
}

 /* log method for client actions, name channel is the name of the channel linked to the thread, ip is the client ip,
  * and flag shows which of the following actions must be written to the log file:
  * 0 quit
  * 1 delete
  * 2 fine vita del thread
  */
void logExit(int flags,char* name_channel, char ip[]){
    int ret;

    file = fopen(NOME_FILE, "a");
    if (file == NULL)ret = -1;
    else ret = 0;

    ERROR_HELPER(ret, "Errore apertura file log.txt");

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    char* date = ctime(&mytime);
    
    switch(flags) {
        case 0:
            ret = fprintf(file, "%sClient with IP %s decided to exit channel named %s\n\n\n", date, ip, name_channel);
            break;
        case 1:
            ret = fprintf(file, "%sClient with IP %s, owner of the channel %s, decided to delete the channel \n\n\n", date, ip, name_channel);
            break;
        case 2:
            ret = fprintf(file, "%sClient with IP %s close the connection\n\n\n", date, ip);
            break;
        default:
            break;
    }
    ERROR_HELPER(ret, "Errore scrittura operazione server su file");
    
    ret = fclose(file);
    ERROR_HELPER(ret, "Errore chiusura file");
}

void resetLog() {
    int ret;
    //	cancella tutto il file di log
    FILE * f;
    //se il file esiste lo resetto
    if ((f = fopen(NOME_FILE, "r")) > 0) {
        ret = remove(NOME_FILE);
        ERROR_HELPER(ret, "Errore cancellazione file");
    }


}
