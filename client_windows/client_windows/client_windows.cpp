// client_windows.cpp : definisce il punto di ingresso dell'applicazione console.
//

/*

TCPCLIENT.C
==========

*/


#include <stdio.h>
#include <winsock.h>
#include <stdlib.h>

#include "common.h"

///#include "helper.h"           /*  Our own helper functions  */

/*  Global constants  */

#define MAX_LINE           (1000)
#define MAX_CHAR           (1000)


SOCKET    socket_desc;           /*  connection socket         */
long port;                  /*  port number               */
struct    sockaddr_in servaddr;  /*  socket address structure  */
char     *szAddress;             /*  Holds remote IP address   */
char     *szPort;                /*  Holds remote port         */
char     *endptr;                /*  for strtol()              */
struct	  hostent *he;



int  myhandler(int event) {
	printf("received CTRL+C\n");
	if (szAddress!=NULL)	free(szAddress);
	if (szPort != NULL)		free(szPort);
	if (endptr != NULL)		free(endptr);	
	ExitProcess(0);
}

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
		
		if (recv_bytes > 0) {
			//printf("\033[%d;%dH", x, y);
			int i;
			for (i = 0; i < recv_bytes; i++) convertitoreUW(&msg_recv[i]);//printf("ricevi %c=%d ", msg_recv[i], msg_recv[i]);
			printf("ricevuto: %s\n", msg_recv);
		
		}
		sum += recv_bytes;
	}
	return (void*)sum;
}

void* Invia(int socket_desc) {	
	int ret;
	char buf[1024];
	unsigned int sum = 0;
	while (1) {
		
		printf("send: ");
		fgets(buf, 1024, stdin); //fgets prende anche il carattere invio
		size_t buf_len = strlen(buf);
		//printf("%d\n",buf_len);
		if (buf_len == 1) continue;
		buf[buf_len - 1] = '\0';
		int i;
	
		//for (i = 0; i < buf_len - 1; i++) 	convertitoreUA(buf[i]);//printf("%c=%d\n", buf[i], buf[i]);
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
	HANDLE thread_recv;
	HANDLE thread_send;
	DWORD id_recv;
	DWORD id_send;

	u_long    nRemoteAddr;
	WSADATA   wsaData;


	he = NULL;

	int ret;


	SetConsoleCtrlHandler((PHANDLER_ROUTINE)myhandler, 1);

	/*  Get command line arguments  */

	//ParseCmdLine(argc, argv, &szAddress, &szPort);
	szAddress = "192.168.1.73";
	szPort = "2016";

	/*  Set the remote port  */

	port = strtol(szPort, &endptr, 0);
	if (*endptr){
		ERROR_HELPER(-1, "client: porta non riconosciuta");
	}


	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0){
		ERROR_HELPER(-1, "errore in WSAStartup()");
	}

	/*  Create the listening socket  */

	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		ERROR_HELPER(-1, "client: errore durante la creazione della socket.");
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
			ERROR_HELPER(-1, "client: errore durante la gethostbyname.");
		}
		printf("riuscita.\n\n");
		nRemoteAddr = *((u_long *)he->h_addr_list[0]);
	}
	servaddr.sin_addr.s_addr = nRemoteAddr;


	/*  connect() to the remote echo server  */
	if (ret=connect(socket_desc, (struct sockaddr *) &servaddr, sizeof(servaddr)) == SOCKET_ERROR){
		//printf("client: errore durante la connect.\n");
		ERROR_HELPER(-1, "errore connect");
	}

	printf("client: connesso al server.\n");

	thread_recv = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)Ricevi,
		(LPVOID)socket_desc,
		NORMAL_PRIORITY_CLASS,
		&id_recv);

	if (thread_recv == NULL) {
		ERROR_HELPER(-1, "errore thread");
	}

	thread_send = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)Invia,
		(LPVOID)socket_desc,
		NORMAL_PRIORITY_CLASS,
		&id_send);

	if (thread_send == NULL) {
		ERROR_HELPER(-1, "errore thread");
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

//converte le lettere da unix a windows
int convertitoreUW(char* n) {	
	if (*n == -61 && *(n + 1) == -87) { //é
		*n = -126;
		*(n + 1) = ' ';
	}
	if (*n == -61 && *(n + 1) == -88) { //è
		*n = -118;
		*(n + 1) = ' ';
	}
	if (*n == -61 && *(n + 1) == -96) { //à
		*n = -123;
		*(n + 1) = ' ';
	} 
	if (*n == -61 && *(n + 1) == -71) { //ù
		*n = -105;
		*(n + 1) = ' ';
	}
	if (*n == -61 && *(n + 1) == -78) { //ò
		*n = -107;
		*(n + 1) = ' ';
	}
	if (*n == -61 && *(n + 1) == -84) { //ì
		*n = -115;
		*(n + 1) = ' ';
	}
	if (*n == -62 && *(n + 1) == -93) { //£
		*n = -100;
		*(n + 1) = ' ';
	}
	if (*n == -62 && *(n + 1) == -80) { //°
		*n = -8;
		*(n + 1) = ' ';
	}
	if (*n == -61 && *(n + 1) == -89) { //ç
		*n = -121;
		*(n + 1) = ' ';
	}
	if (*n == -62 && *(n + 1) == -89) { //§
		*n = -11;
		*(n + 1) = ' ';
	}
}

