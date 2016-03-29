CC = gcc -Wall -O1 -g
LDFLAGS = -lpthread

all: server


server: server.c common.h log.h log.c
	$(CC) -o server server.c log.c $(LDFLAGS)


:phony
clean:
	rm -f server
