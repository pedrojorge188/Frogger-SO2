#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define TAM 40

#define MAX_THREADS 1
#define CMD_NOT_FOUND TEXT("COMMAND NOT FOUND !\n")

typedef struct thParams {
	HANDLE mutex;
}thParams;

void UNICODE_INITIALIZER();
DWORD __stdcall input_thread(LPVOID lpParam);

#endif