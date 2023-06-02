#ifndef FROG_H
#define FROG_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "..\\..\\utils.h";

#define TAM 40

#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define TIMEOUT 10000
#define FROG_RUNNING_MSG TEXT("Já existe dois programas frog em atividade!")
#define FROG_SEMAPHORE TEXT("FrogInstances")

#define N_TRACKS TEXT("FroggerGame\\NumberTracks")

#define START_SPEED TEXT("FroggerGame\\StartSpeed")

#define MAX_FROGS 2

typedef struct thParams {
	CRITICAL_SECTION critical;
	HANDLE pipe;
	HWND mainWindow;
	int status;
	game gameView;
	int myPoints;
	HANDLE receiver;
	wchar_t oldTable[10][20];
}thParams;


int out_flag = 0;
void paint_game_zone(HDC hdc, RECT rect);
void UNICODE_INITIALIZER();
HBITMAP LoadBitmap(int n);

#endif