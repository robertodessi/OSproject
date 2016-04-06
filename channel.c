#include "common.h"
#include "channel.h"


#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()
#include <sys/types.h>


//====================================
//			CHANNEL
//====================================



void* channel_handler(void* arg) {
    handler_args_c* args = (handler_args_c*)arg;
    if(DEBUG) printf("canale\n");
    
    /**TODO: chiude la pipe in scrittura **/
    
    /**TODO:apre la fifo e si mette in ascolto continuo. Gestirà i comandi di join, quit e delete **/
}

void printChannel(channel_struct* channel){
	printf("\nCHANNEL\n");
	printf("id: %d\n",channel->id);
	printf("owner: %d\n",channel->owner);
	printf("dimension: %d\n",channel->dim);
	printf("client_desc: ");
	for(int i=0;i<channel->dim;i++)printf("%d, ",channel->client_desc[i]);
	printf("\n\n");
}
