#include "common.h"
#include "thread_util.h"
#include "console_thread.h"
#include <string.h>
#include <pthread.h>

//showlist
//quit
//nclient= <int>
//nchannel= <int>
//close <channel>


void* console(void*channel_list){
	int quit=0;
	char buf[1024];
	while(!quit){
		printf("console= ");
		fgets(buf, 1024, stdin); 
		//if(strcmp(buf,"quit")) quit=1;
		if(strcmp(buf,"showlist")){
			sem_wait(sem);
			printList((channel_list_struct*)channel_list);
			sem_post(sem);
		 }
		 
		if(strcmp(buf,"kill")){
			sem_wait(sem);
			exit(0);
			sem_post(sem);
		 }
		//if(strcmp(buf,""))
	}
	printf("chiusura...\n");
}
