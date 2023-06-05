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
#define FROG_RUNNING_MSG TEXT("J� existe dois programas frog em atividade!")
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
	game prevGameView;
	int bitmap;
	HDC dBuffer;
	BITMAP bmp[7];
	HBITMAP hBmp[7];
	HDC bmpDC[7];
}thParams;


int out_flag = 0;
void paint_game_zone(HDC* bmpDC, HBITMAP* hBmp, BITMAP* bmp, RECT * rect, HDC hdcBuffer, api receive);
void UNICODE_INITIALIZER();
HBITMAP LoadBitmap(int n);

#endif