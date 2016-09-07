#include "thread_util.h"
#include <pthread.h>
#include <errno.h>//per poter leggere il valore di errno


int invio(char* s, int dest) {
    int ret;
    int left_bytes = sizeof (char)*(strlen(s) + 1);
    int sent_bytes = 0;
    while (left_bytes > 0) {
#ifdef __linux__
        ret = send(dest, s + sent_bytes, left_bytes, MSG_CONFIRM);
#endif
#ifdef  __APPLE__
        ret = send(dest, s + sent_bytes, left_bytes, 0);
#endif
        if (ret < 0 && errno == EINTR) continue;
        if (ret < 0) return -1; //error: return -1;
        sent_bytes += ret;
        left_bytes -= ret;
    }
    return 0;
}

int ricevi(char* buf, size_t buf_len, int mitt, int id_coda, mymsg* recv_message, int* is_connect, sem_t* my_named_semaphore, channel_struct* my_channel) {
    int ret, shouldStop = 0;
    int recv_bytes = 0;

    struct timeval timeout;
    fd_set read_descriptors;
    int nfds = mitt + 1;
	
	time_t start,end;
	start=time(NULL);
	//printf("%ld\n",start);
    
    while (!shouldStop) {
        // check every 1.5 seconds 

        timeout.tv_sec = 1;
        timeout.tv_usec = 500000;

        FD_ZERO(&read_descriptors);
        FD_SET(mitt, &read_descriptors);


        ret = select(nfds, &read_descriptors, NULL, NULL, &timeout);


        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) return -1;

        
        int temp = leggiMSG(id_coda, recv_message);

        //controllo periodicamente se è arrivato qualche messaggio
        if ( temp == 1 ) {	
			//if(strcmp(recv_message->mtext,"killthemall\0")==0) ret = msgctl(id_coda, IPC_RMID, 0);
			//printf("sono %d ricevuto msg tipo %d testo %s\n", mitt, recv_message->mtype, recv_message->mtext);	
			esci(is_connect, my_named_semaphore, my_channel, mitt);
						//if(strcmp(recv_message->mtext,"killthemall\0")==0) return -3; //se il proprietario mi dice che devo uccidere il thread ritorna -3
        }else if ( temp == 3) {  
			//printf("sono %d ricevuto msg tipo %d testo %s\n", mitt,recv_message->mtype, recv_message->mtext);      
            return -3;   //se il mainprocess invia un messaggio di tipo 3  ritorna -2
        }
        
        end=time(NULL);
        
        if((end-start)>(MINUTES)*60){
			printf("timeout occured\n");
			invio("sei stato disconnesso per inattività\0",mitt);
			return -1;
		}
        if (ret == 0) continue; // timeout expired
        
        start=time(NULL);  //se arriva un messaggio resetto start
        
        printf("è arrivato qualcosa\n");

        // ret is 1: read available data!
        int flag = 1;
        while (flag) {
            ret = recv(mitt, buf + recv_bytes, buf_len - recv_bytes, 0);
            if (ret < 0 && errno == EINTR) continue;
            if (ret < 0) return -1; //error: return -1
            recv_bytes += ret;
            if (recv_bytes > 0 && (buf[recv_bytes - 1] == '\0' || recv_bytes>MAX_BYTES)) {
                flag = 0;
            }
            if (recv_bytes == 0)break;
        }
        //int i;
        //for(i=0;i<recv_bytes-1;i++)printf("%c=%d\n",buf[i],(int)buf[i]);
        recv_bytes--;
        shouldStop = 1;

    }

    return recv_bytes;
}

int leggiMSG(int id_coda, mymsg* recv_message) {
    //utilizzo il tipo 1 per i messaggi di delete
    //2 per ack
    //3 per sigterm

    if (msgrcv(id_coda, recv_message, sizeof (mymsg), 3 , IPC_NOWAIT) != -1){ 
        return 3;  //messaggio 3 ricevuto
    } else if (errno != ENOMSG) { //ENOMSG: IPC_NOWAIT asserted, and no message exists in the queue to satisfy the reques
		return -1; //errore!!
    }
    
    if (msgrcv(id_coda, recv_message, sizeof (mymsg), 1, IPC_NOWAIT) != -1) {
        return 1; //messaggio  1 ricevuto			
    } else if (errno != ENOMSG) { //ENOMSG: IPC_NOWAIT asserted, and no message exists in the queue to satisfy the reques
		return -1; //errore!!
    }
    return 0; //nessun messaggio ricevuto
}

int esci(int* is_connect, sem_t* my_named_semaphore, channel_struct* my_channel, int client_desc) {
    //il canale sta per essere eliminato quindi esco
    //if (DEBUG)printf("asked service of type %ld - receive %s\n", recv_message.mtype, recv_message.mtext);
    *is_connect = 0;
    if (sem_close(my_named_semaphore) == -1) {
        return -1;
    }
    
	//avverto il proprietario di aver fatto la sem_close!!
	mymsg msg;
	msg.mtype = 2; //header del messaggio. 1:delete  2:sem_close 3:kill
	strcpy(msg.mtext, "semclose\0");

	int id_coda_other = msgget(my_channel->client_desc[0], IPC_EXCL | 0666); //prendo la coda di messaggi del proprietario...
	if (id_coda_other == -1) {
		printf("cannot open server queue, please check with the problem\n");
		return -1;
	}
	if (msgsnd(id_coda_other, &msg, SIZE, FLAG) == -1) { //...gli invio il messaggio
		printf("cannot return response to the client\n");
		return -1;
	} 
	//if (DEBUG)printf("sono %d ricevuto msg tipo %d testo %s\n", client_desc,msg.mtype,msg.mtext);

	my_channel = NULL;
    invio("sei stato disconnesso dal canale\0", client_desc); //avverto il client che è stato disconnesso dal canale
    return 0;
}


char* prendiNome(char* str, int len, size_t command_len) {
    char* res = (char*) malloc(sizeof (char) * (len - command_len));
    int index = command_len;
    int i = 0;
    while (str[index] == ' ') index++; //tolgo gli eventuali spazi tra il comando e il nome del canale  
    for (; i < len; i++) res[i] = str[index++];
    res[++i] = '\0';
    return res;
}



void printList(channel_list_struct* list) {
    int i;
    for (i = 0; i < list->num_channels; i++) {
        printChannel(list->channel[i]);
        printf("\n");
    }
}

void printChannel(channel_struct* channel) {
    printf("\nCHANNEL\n");
    printf("name: %s\n", channel->name_channel);
   // printf("id: \n");
    //for (int i = 0; i < channel->dim; i++) printf("%d, ", channel->id[i]);
    printf("\n");
    printf("owner: %d\n", channel->owner);
    printf("dimension: %d\n", channel->dim);
    printf("client_desc: ");
    for (int i = 0; i < channel->dim; i++)printf("%d, ", channel->client_desc[i]);
    printf("\n\n");
}
