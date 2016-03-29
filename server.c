#include "common.h"
#include "log.h"

#include <stdio.h>//printf
#include <stdlib.h>//warning exit
#include <errno.h>//per poter leggere il valore di errno
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()

#include <fcntl.h>// serve per le opzioni di apertura del file
#include <time.h>//per giorno e data correnti


//#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)



//====================================
//			MAIN
//====================================

int main(int argc, char *argv[]) {
	
	//	error check descriptor 
    int ret;

    //	socekt descriptor
    int socket_desc;
    
    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0};

    int sockaddr_len = sizeof(struct sockaddr_in); // we will reuse it for accept()
    

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
    
    
    if(DEBUG) resetLog(); //resetto il file di log
    logMsg("Server started to run\n");

	// initialize socket for listening
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    //socket_desc = -1;
    ERROR_HELPER(socket_desc, "Errore creazione socket\n\n");
	
	server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!
    
    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");

    return 0;
}


