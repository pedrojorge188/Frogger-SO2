#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define TAM 40
#define TIMEOUT 10000
#define SERVER_RUNNING_MSG TEXT("Já Existe um servidor em atividade!")

#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define N_TRACKS TEXT("FroggerGame\\NumberTracks")
#define N_TRACKS_ATT TEXT("TRACKS_VALUE")
#define TRACK_COMMAND _T("tracks")
#define COUT_TRACKS TEXT("[SERVER] Insira o número de estradas (1-8) :")
#define GAME_MAX_TRACKS 8

#define START_SPEED TEXT("FroggerGame\\StartSpeed")
#define START_SPEED_ATT TEXT("SPEED_VALUE")
#define COUT_SPEED TEXT("[SERVER] Insira a velocidade inicial dos carros :")
#define SPEED_COMMAND _T("speed")

#define MAX_THREADS 1
#define CMD_NOT_FOUND TEXT("COMMAND NOT FOUND !\n")

typedef struct game {	
	INT num_tracks;
	INT vehicle_speed;
}game;

typedef struct thParams {
	HANDLE mutex;
	game * gameData;
}thParams;

void UNICODE_INITIALIZER();
game FillRegistryValues();
DWORD __stdcall input_thread(LPVOID lpParam);
int ChangeNumTracks(INT value);
int ChangeSpeed(INT value);

#endif