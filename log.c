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
int file;

void logMsg(char* toWrite) {

    int ret;
    
    file = open(NOME_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666); 
	ERROR_HELPER(file,"Errore apertura file log.txt");
	
    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    //	gestione segnali
    //sigset_t maschera;
	char* date=ctime(&mytime);
	char msg[strlen(toWrite)+3] ; 
	strcat(msg,toWrite);
	strcat(msg,"\n\n"); //aggiunte 2 \n per rendere il file di log più leggibile
    
    ret = write(file, date, strlen(date));
    ret |= write(file, msg, strlen(msg));
    ERROR_HELPER(ret, "Errore scrittura operazione server su file");
    
    ret=close(file);
    ERROR_HELPER(ret,"Errore chiusura file");

}

void resetLog(){
	int ret;
	//	cancella tutto il file di log
    ret = remove(NOME_FILE);
    ERROR_HELPER(ret, "Errore cancellazione file");
    
}
