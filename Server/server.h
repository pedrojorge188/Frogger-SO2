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


typedef struct game {	
	INT num_tracks;
	INT vehicle_speed;
}game;


void unicodeInitializer();
game fillGameStruct();

#endif