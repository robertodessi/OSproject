#include "thread.h"
#include "common.h"

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()
#include <sys/types.h>


//====================================
//			THREAD
//====================================

void* connection_handler(void* arg) {
    handler_args_t* args = (handler_args_t*) arg;
    channel_list_struct* channel_list = (args->channel_list); //mi salvo la channel_list_struct (per comodità) 
    channel_struct* my_channel;

    int ret, recv_bytes;
    char* name_channel; //nome del canale

    int connect = 0; //flag che indica se il client è connesso ad un canale oppure no

    char buf[1024];
    size_t buf_len = sizeof (buf);

    char msg[1024];
    size_t msg_len;


    /**COMMAND**/

    //CREATE
    char* create_command = CREATE_COMMAND;
    size_t create_command_len = strlen(create_command);

    //JOIN
    char* join_command = JOIN_COMMAND;
    size_t join_command_len = strlen(join_command);

    //QUIT
    char* quit_command = QUIT_COMMAND;
    size_t quit_command_len = strlen(quit_command);

    /**END_COMMAND **/


    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(args->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short

    while (1) {

        // read message from client
        while ((recv_bytes = recv(args->socket_desc, buf, buf_len, 0)) < 0) {
            if (errno == EINTR) continue;
            ERROR_HELPER(-1, "Cannot read from socket");
        }

        /** TODO: distinguere dai comandi i messaggi scritti dal client ed inoltrarli a tutti gli altri client connessi nel canale*/

        /**TODO: fare il log**/

        // check if create 
        /**  /create <name_channel>  **/
        if (!connect && !memcmp(buf, create_command, create_command_len)) {
            if (DEBUG) printf("try to create new channel\n");

            //name_channel=prendiNome(buf); //prendo il nome del canale
            name_channel = prendiNome(buf, recv_bytes + 1); //prendo il nome del canale


            //accedo alla lista condivisa
            ret = sem_wait(sem);
            ERROR_HELPER(ret, "error sem_wait");

            /**TODO: verificare che il nome del canale sia disponibile**/

            ret = sem_post(sem);
            ERROR_HELPER(ret, "error sem_post"); //fine sezione critica


            if (DEBUG) printf("create new channel\n");

                channel_struct* my_channel = (channel_struct*) malloc(sizeof (channel_struct));
                my_channel -> dim = 1; //quando si crea il canale c'è solo il proprietario
                my_channel -> client_desc = (int*) malloc(sizeof (int)); //allocazione dinamica
                my_channel -> client_desc[0] = args->socket_desc; //aggiungo il proprietario ai client connessi al canale
                my_channel -> owner = args->socket_desc; //setto il proprietario
                my_channel -> name_channel = name_channel;
                my_channel -> id = 0; //setto un id al canale. Per ora a tutti zero 		


                //accedo alla lista condivisa
                ret = sem_wait(sem);
                ERROR_HELPER(ret, "error sem_wait");


                /**	TODO: aggiungere my_channel alla lista dei canali (channel_list)**/

                ret = sem_post(sem);
                ERROR_HELPER(ret, "error sem_post");

                if (DEBUG) printChannel(my_channel);

                connect = 1;

            }
        // check if join
        /**  /join <name_channel>  **/
        if (!connect && recv_bytes == join_command_len && !memcmp(buf, join_command, join_command_len)) {
            printf("join canale\n");
            /**TODO: join al canale**/
            connect = 1;
        }
        //check if quit
        if (connect && recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) {
            printf("quit canale\n");

            //chi esce NON è il propretario del canale
            if (my_channel->owner != args->socket_desc) {

                /**TODO: cancellare da my_channel il suo descrittore*/

            } else {
                /**TODO: gestire in caso in cui il creatore esce dal canale**/
            }

            connect = 0;
        }
        /**TODO: gestire altri comandi (e.g. /quit, /delete, ecc...)**/
    }

    // close socket
    ret = close(args->socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    if (DEBUG) fprintf(stderr, "Thread created to handle the request has completed.\n");

    free(args->client_addr); // do not forget to free this buffer!
    free(args);

    pthread_exit(NULL);
}

char* prendiNome(char* str, int len) {
    char* res = (char*) malloc(sizeof (char)*20);
    //strcpy(res, "prova");
    int index;

    int i = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == " ") {
            index = ++i;
            break;
        }
    }
    for (i = 0; i < len - index; i++) res[i] = str[index++];
    res[++i] = "\0";
    return res;
}

void printChannel(channel_struct* channel) {
    printf("\nCHANNEL\n");
    printf("name: %s\n", channel->name_channel);
    printf("id: %d\n", channel->id);
    printf("owner: %d\n", channel->owner);
    printf("dimension: %d\n", channel->dim);
    printf("client_desc: ");
    for (int i = 0; i < channel->dim; i++)printf("%d, ", channel->client_desc[i]);
    printf("\n\n");
}
