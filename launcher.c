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
#ifdef __linux__
            ret=system("xterm -hold ./client_prova");
#endif
#ifdef  __APPLE__ && __MACH__
            ret = system("open -a Terminal.app ./client_prova");
#endif
        } else {
            // father: server
            ret=system("./server");
        }
    ret++;
	return 0;
}
