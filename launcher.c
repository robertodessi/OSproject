#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 



int main(){
	
	pid_t pid = fork();
	int ret;
	
	if (pid == -1) {
            printf("errore\n");
            exit(-1);
        } else if (pid == 0) {
            // child: client_prova
            ret=system("gnome-terminal -x ./client_prova");
            
        } else {
            // father: server
            ret=system("./server");
        }
    ret++;
	return 0;
}
