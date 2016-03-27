CC = gcc -Wall -O0 -g
LDFLAGS = -lpthread

all: server

server: server.c
	$(CC) -o server server.c $(LDFLAGS)


:phony
clean:
	rm -f server