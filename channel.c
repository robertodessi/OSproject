#include "common.h"
#include "channel.h"

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()

//====================================
//			CHANNEL
//====================================




void* channel_handler(void* arg) {
    handler_args_t* args = (handler_args_t*)arg;
    printf("canale\n");
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
