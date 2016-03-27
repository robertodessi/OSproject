#include <stdio.h>//printf
#include <stdlib.h>//warning exit
#include <string.h>//warning strlen strcat
#include <errno.h>//per poter leggere il valore di errno
#include <netinet/in.h>
#include <netdb.h>

#include <fcntl.h>// serve per le opzioni di apertura del file

#include <time.h>//per giorno e data correnti

#define GENERIC_ERROR_HELPER(cond, errCode, msg)     do {           	\
        if (cond) {                                                 	\
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));	\
            time_t mytime;						\
            mytime = time(NULL);                                        \
            logMsg(file, ctime(&mytime), msg);				\
            exit(EXIT_FAILURE);                                     	\
        }                                                           	\
    } while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
//#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

#define NOME_FILE "log.txt"

#define DEBUG 0

//  file descriptor for server log
int file;

//====================================
//			MAIN
//====================================

int main(int argc, char *argv[]) {

    //	socekt descriptor
    int sock_desc;

    //	error check descriptor 
    int ret;

    if ((file = open(NOME_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1) {
        fprintf(stderr, "Errore apertura file di log\n");
        exit(EXIT_FAILURE);
    }

    //	cancella tutto il file di log
    if (DEBUG) {
        ret = remove(NOME_FILE);
        ERROR_HELPER(ret, "Errore cancellazione file");
    }

    //	getting current time and date
    time_t mytime;
    mytime = time(NULL);

    //	gestione segnali
    sigset_t maschera;

    /*
    ret = sigfillset(&maschera);
    ERROR_HELPER(ret, "Errore nella sigfillset\n\n");

    ret = sigdelset(&maschera, SIGTERM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGTERM)\n\n");

    ret = sigdelset(&maschera, SIGINT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGINT)\n\n");

    ret = sigdelset(&maschera, SIGQUIT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGQUIT)\n\n");

    ret = sigdelset(&maschera, SIGHUP);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGHUP)\n\n");

    ret = sigdelset(&maschera, SIGPIPE);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGPIPE)\n\n");

    ret = sigdelset(&maschera, SIGALRM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGALRM)\n\n");

    ret = sigprocmask(SIG_BLOCK, &maschera, NULL);
    ERROR_HELPER(ret, "Errore nella sigprocmask\n\n");

    gestione_segnali(SIGTERM, gestione_chiusura, 0);
    gestione_segnali(SIGINT, gestione_chiusura, 0);
    gestione_segnali(SIGQUIT, gestione_chiusura, 0);
    gestione_segnali(SIGHUP, gestione_chiusura, 0);
    gestione_segnali(SIGPIPE, gestione_chiusura_inattiva, 0);
    gestione_segnali(SIGALRM, gestione_timer, 0);
     */

    logMsg(file, ctime(&mytime), "Server started to run\n\n");

    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    sock_desc = -1;
    ERROR_HELPER(sock_desc, "Errore creazione socket\n\n");

    return 0;
}

//====================================
//			LOG WRITER
//====================================

int logMsg(int fd, char* date, char* toWrite) {

    int ret;

    ret = write(fd, date, strlen(date));
    ret |= write(fd, toWrite, strlen(toWrite));
    ERROR_HELPER(ret, "Errore scrittura operazione server su file\n\n");

    return ret;
}
