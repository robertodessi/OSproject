CC = gcc -Wall -O1 -g
LDFLAGS = -lpthread

all: server client_prova launcher


server: server.c common.h log.h log.c channel.h channel.c thread.h thread.c
	$(CC) -o server server.c log.c channel.c thread.c $(LDFLAGS)
	
client_prova: client_prova.c common.h 
	$(CC) -o client_prova client_prova.c log.c $(LDFLAGS)
	
launcher: launcher.c
	$(CC) -o launcher launcher.c
	
run:
	./launcher 

:phony
clean:
	rm -f server client_prova
