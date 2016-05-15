// client_windows.cpp : definisce il punto di ingresso dell'applicazione console.
//

/*

TCPCLIENT.C
==========

*/


#include <stdio.h>
#include <winsock.h>
#include <stdlib.h>

///#include "helper.h"           /*  Our own helper functions  */

/*  Global constants  */

#define MAX_LINE           (1000)
#define MAX_CHAR           (1000)

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort);

void* Ricevi(int socket_desc){
	unsigned int sum = 0;
	int ret;
	char msg_recv[1024];
	size_t msg_recv_len = sizeof(msg_recv);
	while (1) {
		int recv_bytes = 0;
		int flag = 1;
		while (flag) {
			ret = recv(socket_desc, msg_recv + recv_bytes, msg_recv_len - recv_bytes, 0);
			if (ret<0 && errno == EINTR) continue;
			if (ret<0) return (void*)sum;  //error: return -1
			recv_bytes += ret;
			if (recv_bytes>0 && msg_recv[recv_bytes - 1] == '\0') {
				flag = 0;
			}
			if (recv_bytes == 0)break;
		}
		if (recv_bytes > 0)printf("ricevuto: %s\n", msg_recv);
		sum += recv_bytes;
	}
	return (void*)sum;
}

void* Invia(int socket_desc) {	
	int ret;
	char buf[1024];

	unsigned int sum = 0;
	while (1) {
		//printf("send: ");
		fgets(buf, 1024, stdin); //fgets prende anche il carattere invio
		size_t buf_len = strlen(buf);
		//printf("%d\n",buf_len);
		if (buf_len == 1) continue;
		buf[buf_len - 1] = '\0';
		//--buf_len; // remove '\n' from the end of the message
		while ((ret = send(socket_desc, buf, buf_len, 0)) < 0) {
			if (errno == EINTR) continue;
			exit(-1);
		}
		sum += ret;
	}
	return (void*)sum;
}

/*  main()  */

int main(int argc, char *argv[])
{
	SOCKET    socket_desc;           /*  connection socket         */
	long port;                  /*  port number               */
	struct    sockaddr_in servaddr;  /*  socket address structure  */
	char     *szAddress;             /*  Holds remote IP address   */
	char     *szPort;                /*  Holds remote port         */
	char     *endptr;                /*  for strtol()              */
	struct	  hostent *he;

	HANDLE thread_recv;
	HANDLE thread_send;
	DWORD id_recv;
	DWORD id_send;

	u_long    nRemoteAddr;
	WSADATA   wsaData;

	he = NULL;

	/*  Get command line arguments  */

	ParseCmdLine(argc, argv, &szAddress, &szPort);


	/*  Set the remote port  */

	port = strtol(szPort, &endptr, 0);
	if (*endptr){
		printf("client: porta non riconosciuta.\n");
		exit(EXIT_FAILURE);
	}


	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0){
		printf("errore in WSAStartup()\n");
		exit(EXIT_FAILURE);
	}

	/*  Create the listening socket  */

	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		fprintf(stderr, "client: errore durante la creazione della socket.\n");
		exit(EXIT_FAILURE);
	}


	/*  Set all bytes in socket address structure to
	zero, and fill in the relevant data members   */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	/*  Set the remote IP address  */
	nRemoteAddr = inet_addr(szAddress);
	if (nRemoteAddr == INADDR_NONE){
		printf("client: indirizzo IP non valido.\nclient: risoluzione nome...");
		if ((he = gethostbyname(szAddress)) == 0){
			printf("fallita.\n");
			exit(EXIT_FAILURE);
		}
		printf("riuscita.\n\n");
		nRemoteAddr = *((u_long *)he->h_addr_list[0]);
	}
	servaddr.sin_addr.s_addr = nRemoteAddr;


	/*  connect() to the remote echo server  */
	if (connect(socket_desc, (struct sockaddr *) &servaddr, sizeof(servaddr)) == SOCKET_ERROR){
		printf("client: errore durante la connect.\n");
		exit(EXIT_FAILURE);
	}


	thread_recv = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)Ricevi,
		(LPVOID)socket_desc,
		NORMAL_PRIORITY_CLASS,
		&id_recv);

	if (thread_recv == NULL) {
		printf("cannot create thread %d\n",id_recv);
		ExitProcess(-1);
	}

	thread_send = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)Invia,
		(LPVOID)socket_desc,
		NORMAL_PRIORITY_CLASS,
		&id_send);

	if (thread_send == NULL) {
		printf("cannot create thread %d\n",id_send);
		ExitProcess(-1);
	}

	WaitForSingleObject(thread_recv, INFINITE);
	WaitForSingleObject(thread_send, INFINITE);

	WSACleanup();
	return EXIT_SUCCESS;
}


int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort){
	int n = 1;

	while (n < argc){
		if (!strncmp(argv[n], "-a", 2) || !strncmp(argv[n], "-A", 2)){
			*szAddress = argv[++n];
		}
		else
			if (!strncmp(argv[n], "-p", 2) || !strncmp(argv[n], "-P", 2)){
				*szPort = argv[++n];
			}
			else
				if (!strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2)){
					printf("Sintassi:\n\n");
					printf("    client -a (indirizzo server) -p (porta del server) [-h].\n\n");
					exit(EXIT_SUCCESS);
				}
		++n;
	}
	if (argc == 1){
		printf("Sintassi:\n\n");
		printf("    client -a (indirizzo server) -p (porta del server) [-h].\n\n");
		exit(EXIT_SUCCESS);
	}
	return 0;
}