#include "common.h"
#include "thread_util.h"

#include <string.h>
#include <stdio.h>

void* console(void* arg){
	int quit=0;
	int n=0;
	char buf[1024];
	while(!quit){
		printf("console>");
		fgets(buf,1024,stdin);
		printf("%s %d\n",buf,strlen(buf));
		if(strcmp(buf,"printList\0")==0){
			printf("1\n");
			printList((channel_list_struct*)arg);
			
		}
		if(strcmp(buf,"set max_client\0")==0){
			printf("2\n");
			sem_wait(sem);
			
			#ifdef __STDC_LIB_EXT1__
			int ret = scanf("max_client=%d",&n,sizeof(int));
			ret=ret;
			#endif
			
			max_client=n;
			sem_post(sem);
			
		}
		if(strcmp(buf,"quit")==0){
			printf("3\n");
			quit=1;
			
		}
		
	
	}
	
	printf("chiudendo...\n");
	pthread_exit(NULL);
	
}
