#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define TAM 40
#define N_TRACKS TEXT("FroggerGame\\NumberTracks")
#define N_TRACKS_ATT TEXT("TRACKS_VALUE")
#define COUT_TRACKS TEXT("[SERVER] Insira o número de estradas (1-8) :")
#define GAME_MAX_TRACKS 8

#define START_SPEED TEXT("FroggerGame\\StartSpeed")
#define START_SPEED_ATT TEXT("SPEED_VALUE")
#define COUT_SPEED TEXT("[SERVER] Insira a velocidade inicial dos carros :")

#define MAX_THREADS 1

typedef struct game {	
	INT num_tracks;
	INT vehicle_speed;
}game;

typedef struct thParams {
	int out_flag;
}thParams;

void UNICODE_INITIALIZER();
game FillRegistryValues();

#endif