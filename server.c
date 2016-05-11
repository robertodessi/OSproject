#include "common.h"
#include "log.h"
#include "thread.h"


#include <stdio.h>//printf
#include <stdlib.h>//warning exit
#include <errno.h>//per poter leggere il valore di errno
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <unistd.h> 
#include <pthread.h>
#include <signal.h> //signals handling


//	socekt descriptor
int socket_desc, client_desc;

void error_handling(int signal, void (*handler)(int)) {
    struct sigaction act;
    act.sa_handler = handler;
    sigaction(signal, &act, NULL);
}

void safe_exit() {
    logMsg("\nServer shutting down, signal caught");
    printf("Server shutting down\n");
    fflush(stdout);
    close(socket_desc);

    // TODO FARE LE FREE

    exit(0);
}

// TODO da usare

void gestione_sigsegv(int dummy1, siginfo_t *info, void *dummy2) {
    unsigned int address;
    address = (unsigned int) info->si_addr;
    printf("segfault occurred (address is %x)\n", address);
    fflush(stdout);
}

sigset_t mask;

// TODO da usare

void sigsegv() {
    struct sigaction act;
    act.sa_sigaction = gestione_sigsegv;
    act.sa_mask = mask;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, NULL);
}

void brokenPipe() {
    logMsg("BROKEN PIPE");
    printf("BROKEN PIPE\n");
    fflush(stdout);
}


//====================================
//			MAIN
//====================================

int main(int argc, char *argv[]) {

    //	error check descriptor 
    int ret;

    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0};

    int sockaddr_len = sizeof (struct sockaddr_in); // we will reuse it for accept()

    //struttura che rappresenta la lista di tutti i canali
    channel_list_struct* channel_list = (channel_list_struct*) malloc(sizeof (channel_list_struct));
    channel_list->num_channels = 0; //inizialmente ci sono 0 canali
    channel_list->name_channel = (char**) malloc(0); //inizializzo le strutture dati
    channel_list->channel = (channel_struct**) malloc(0); //inizializzo le strutture dati

    //alloco e inizializzo il semaforo
    sem = (sem_t*) malloc(sizeof (sem_t));
    //ret=sem_open(sem, 0, 1);

    sem = sem_open(NAME_SEM, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED && errno == EEXIST) {
        printf("already exists, let's unlink it...\n");
        sem_unlink(NAME_SEM);
        printf("and then reopen it...\n");
        sem = sem_open(NAME_SEM, O_CREAT | O_EXCL, 0666, 1);
    }
    if (sem == SEM_FAILED) {
        ERROR_HELPER(-1, "[FATAL ERROR] Could not create a semaphore");
    }

    ret = sigemptyset(&mask);
    ERROR_HELPER(ret, "Errore nella sigemptyset\n\n");

    ret = sigfillset(&mask);
    ERROR_HELPER(ret, "Errore nella sigfillset\n\n");

    ret = sigdelset(&mask, SIGTERM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGTERM)\n\n");

    ret = sigdelset(&mask, SIGINT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGINT)\n\n");

    ret = sigdelset(&mask, SIGQUIT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGQUIT)\n\n");

    ret = sigdelset(&mask, SIGHUP);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGHUP)\n\n");

    ret = sigdelset(&mask, SIGPIPE);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGPIPE)\n\n");

    ret = sigdelset(&mask, SIGALRM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGALRM)\n\n");

    ret = pthread_sigmask(SIG_BLOCK, &mask, NULL);
    ERROR_HELPER(ret, "Errore nella pthread_sigmask\n\n");

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    error_handling(SIGTERM, safe_exit);
    error_handling(SIGINT, safe_exit);
    error_handling(SIGQUIT, safe_exit);
    error_handling(SIGHUP, safe_exit);
    error_handling(SIGILL, safe_exit);
    error_handling(SIGPIPE, brokenPipe);

    //error_handling(SIGPIPE, brokenPipe);



    //Presentation
    printf("Server: Welcome by ChatApp!\n");

    resetLog(); //resetto il file di log
    logMsg("Server started to run");

    // initialize socket for listening
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    //socket_desc = -1;
    ERROR_HELPER(socket_desc, "Errore creazione socket\n\n");

    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); // don't forget about network byte order!

    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof (reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");

    // we allocate client_addr dynamically and initialize it to zero
    struct sockaddr_in* client_addr = calloc(1, sizeof (struct sockaddr_in));


    // loop to manage incoming connections forking the server process
    while (1) {
        // accept incoming connection
        client_desc = accept(socket_desc, (struct sockaddr*) client_addr, (socklen_t*) & sockaddr_len);
        if (client_desc == -1 && errno == EINTR) continue; // check for interruption by signals
        if (client_desc == -1) {
            printf("Cannot open socket for incoming connection\n");
            continue;
        }

        // parse client IP address and port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
        uint16_t client_port = ntohs(client_addr->sin_port); // port number is an unsigned short

        if (DEBUG) fprintf(stderr, "Incoming connection accepted... %s:%d\n", client_ip, client_port);

        logConnection(client_ip, client_port);

        //creo il thread che gestirà il client da ora in avanti
        pthread_t thread;

        // put arguments for the new thread into a buffer
        handler_args_t * thread_args = (handler_args_t*) malloc(sizeof (handler_args_t));
        thread_args -> socket_desc = client_desc; //passo il descrittore del client
        thread_args -> client_addr = client_addr; //passo l'indirizzo del client
        thread_args -> channel_list = channel_list; //passo il puntatore alla lista dei canali


        if (pthread_create(&thread, NULL, connection_handler, (void*) thread_args) != 0) {
            fprintf(stderr, "Can't create a new thread, error %d\n", errno);
            exit(EXIT_FAILURE);
        }

        if (DEBUG) fprintf(stderr, "New thread created to handle the request!\n");

        pthread_detach(thread); // I won't phtread_join() on this thread	

        // we can't just reset fields: we need a new buffer for client_addr!
        client_addr = calloc(1, sizeof (struct sockaddr_in));

    }

    exit(EXIT_SUCCESS); // this will never be executed

}

//lo appunto qui perchè così me lo ricordo  http://www.dis.uniroma1.it/~aniello/sc/m2/es10/sol/chat_socket.c

/* select() uses sets of descriptors and a timeval interval. The
 * methods returns when either an event occurs on a descriptor in
 * the sets during the given interval, or when that time elapses.
 *
 * The first argument for select is the maximum descriptor among
 * those in the sets plus one. Note also that both the sets and
 * the timeval argument are modified by the call, so you should
 * reinitialize them across multiple invocations.
 *
 * On success, select() returns the number of descriptors in the
 * given sets for which data may be available, or 0 if the timeout
 * expires before any event occurs. 
 */

