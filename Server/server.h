#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define TAM 40

#define TIMEOUT 10000
#define SERVER_LIMIT_USERS 1
#define SERVER_RUNNING_MSG TEXT("J� Existe um servidor em atividade!")
#define SERVER_SEMAPHORE TEXT("ServerInstances")

#define N_TRACKS TEXT("FroggerGame\\NumberTracks")
#define N_TRACKS_ATT TEXT("TRACKS_VALUE")
#define TRACK_COMMAND _T("tracks")
#define COUT_TRACKS TEXT("[SERVER] Insira o n�mero de estradas (1-8) :")
#define GAME_MAX_TRACKS 8

#define START_SPEED TEXT("FroggerGame\\StartSpeed")
#define START_SPEED_ATT TEXT("SPEED_VALUE")
#define COUT_SPEED TEXT("[SERVER] Insira a velocidade inicial dos carros :")
#define SPEED_COMMAND _T("speed")

#define MAX_THREADS 2
#define CMD_NOT_FOUND TEXT("COMMAND NOT FOUND !\n")

#define MAX_FROGS 2
#define W_GAME 20 //largura da area de jogo (COLUNAS)
#define H_GAME 10 //altura da area de jogo (LINHAS)
#define MAX_VEHICLES 8

int out_flag = 0;

typedef struct frog {
	int x, y;
	int points; // se -1 perdeu o jogo e � desconectado
}frog;

typedef struct vehicles {
	int x, y;
	int orientation; // 1-> direita para a esquerda, 2-> esquerda para a direita
} vehicles;

typedef struct game {
	frog frogs[MAX_FROGS];			//Sapos (Clientes)
	vehicles cars[8][MAX_VEHICLES]; // veiculos[pos na faxa][o id do carro]
	int mode;				// 1-> Individual 2-> Competi��o
	INT num_tracks;			// Numero de estradas
	INT vehicle_speed;	   // Velocidade dos carros
	int n_cars_per_track; // Random de numero de carros por track
	char table[H_GAME][W_GAME]; // Tabela de vizualiza��o para debug
}game;

typedef struct thParams {
	HANDLE mutex;
	game * gameData;
}thParams;

void UNICODE_INITIALIZER();
game FillRegistryValues();
int FillGameDefaults(game * g);
void moveCars(game * g);
DWORD __stdcall input_thread(LPVOID lpParam);
int ChangeNumTracks(INT value);
int ChangeSpeed(INT value);

#endif