// client_windows.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"



#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define ECHOING_EXIT_CODE 2

int main(int argc, char *argv[]) {

	BOOL newprocess;
	STARTUPINFO si;
	PROCESS_INFORMATION  pi;
	DWORD exitcode;
	char buff[128];

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	if (argc == 2) {
		printf("Starting up echoing process ....\n");
		fflush(stdout);
		newprocess = CreateProcessA((LPCSTR)argv[1], (LPSTR)argv[1], NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, (LPSTARTUPINFO)&si, &pi);
		if (newprocess == FALSE) {
			printf(" failure\n\n");
			ExitProcess(-1);
		};
		printf(" done\n\n");
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &exitcode);
		printf("\nEchoing process exited with code %d\n", exitcode);
		fflush(stdout);

	}
	else {
		printf("\tHere I'm for echoing\n\t");
		fflush(stdout);
		while (1) {
			scanf("%s", buff);
			if (strcmp(buff, "bye") == 0) ExitProcess(ECHOING_EXIT_CODE);
			printf("\t%s\n\t", buff);
			fflush(stdout);
		}
	}
}

