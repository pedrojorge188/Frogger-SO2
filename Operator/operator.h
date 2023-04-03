#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define TAM 40
#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define MAX_THREADS 2
#define CMD_NOT_FOUND TEXT("COMMAND NOT FOUND !\n")

int out_flag;

typedef struct thParams {
	HANDLE console;
	HANDLE mutex;
	COORD cinput;
	COORD coutput;
}thParams;

void UNICODE_INITIALIZER();
DWORD __stdcall input_thread(LPVOID lpParam);

#endif