CC = gcc -Wall -O1 -g 
LDFLAGS = -lpthread

all: server client_prova launcher

 
server: server.c common.h log.h log.c thread.h thread.c thread_util.h thread_util.c console_thread.h console_thread.c
	$(CC) -o server server.c log.c thread.c thread_util.c console_thread.c $(LDFLAGS)
	
client_prova: client_prova.c common.h 
	$(CC) -o client_prova client_prova.c log.c $(LDFLAGS)
	
launcher: launcher.c
	$(CC) -o launcher launcher.c
	
run:
	./launcher 

:phony
clean:
	rm -f server client_prova launcher
