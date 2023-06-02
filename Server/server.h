#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "..\\utils.h"

#define TAM 40

#define TIMEOUT 10000
#define SERVER_LIMIT_USERS 1
#define SERVER_RUNNING_MSG TEXT("Já Existe um servidor em atividade!")

#define N_TRACKS TEXT("FroggerGame\\NumberTracks")
#define N_TRACKS_ATT TEXT("TRACKS_VALUE")
#define TRACK_COMMAND _T("tracks")
#define COUT_TRACKS TEXT("[SERVER] Insira o número de estradas (1-8) :")
#define GAME_MAX_TRACKS 8

#define START_SPEED TEXT("FroggerGame\\StartSpeed")
#define START_SPEED_ATT TEXT("SPEED_VALUE")
#define COUT_SPEED TEXT("[SERVER] Insira a velocidade inicial dos carros :")
#define SPEED_COMMAND _T("speed")

#define MAX_THREADS 2
#define CMD_NOT_FOUND TEXT("COMMAND NOT FOUND !\n")

#define DEFAULT 2

int out_flag = 0;

typedef struct moveParam{

	int track;
	game* gameData;
	CRITICAL_SECTION critical;
	HANDLE updateEvent;
	int frogs_connected;
	HANDLE overlapEvents[2];
	PipeData hPipes[2];
}moveParam;

typedef struct thParams {

	HANDLE * thIDs;
	HANDLE * move_threads;
	game * gameData;
	HANDLE hWrite;
	HANDLE hRead;
	CRITICAL_SECTION critical;
	HANDLE updateEvent;
	HANDLE overlapEvents[2];
	PipeData hPipes[2];
	int frogs_connected;

}thParams;

void removeFrog(game* g , int id);
void WritePipe(PipeData* p, game* g);
void moveUp(game* g, int id);
void moveRight(game* g, int id);
void moveLeft(game* g, int id);
void setFrog(game* g, int id);
void moveCars(thParams * p);
void UNICODE_INITIALIZER();
game FillRegistryValues();
int FillGameDefaults(game * g);
int setObstacle(game* g);
DWORD __stdcall input_thread(LPVOID lpParam);
int ChangeNumTracks(INT value);
int ChangeSpeed(INT value);
void invertOrientation(game* g);
void moveDown(game* g, int id);

#endif