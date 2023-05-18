#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define SERVER_SHUTDOWN TEXT("ServerExit")
#define SHARED_MUTEX TEXT("SHARED_MUTEX")
#define WRITE_SEMAPHORE  TEXT("SEMAFORO_ESCRITA")
#define READ_SEMAPHORE  TEXT("SEMAFORO_LEITURA")
#define MUTEX_COMMAND_ACCESS TEXT("MUTEX_COMANDOS")
#define PIPE_NAME TEXT("\\\\.\\pipe\\listener")
#define BUFFER_SIZE 50
#define UPDATE_EVENT TEXT("UPDATE_EVENT")
#define MAX_FROGS 2
#define W_GAME 20 //largura da area de jogo (COLUNAS)
#define H_GAME 10 //altura da area de jogo (LINHAS)
#define MAX_VEHICLES 8
#define N_CLIENTS 2

typedef struct PipeData {

	int id;
	HANDLE hPipe;
	OVERLAPPED overlap;
	BOOL active;

}PipeData;

typedef struct frog {
	int x, y;
	int points; // se -1 perdeu o jogo e é desconectado
}frog;

typedef struct vehicles {
	int x, y;
	int orientation; // 1-> direita para a esquerda, 2-> esquerda para a direita
} vehicles;

typedef struct game {

	frog frogs[2];			//Sapos (Clientes)
	vehicles cars[8][8]; // veiculos[pos na faxa][o id do carro]
	int mode;				// 1-> Individual 2-> Competição
	INT num_tracks;			// Numero de estradas
	INT vehicle_speed;	   // Velocidade dos carros
	int track_speed[8];
	int n_cars_per_track; // Random de numero de carros por track
	wchar_t table[H_GAME][W_GAME]; // Tabela de vizualização

}game;

typedef struct api_pipe {

	int mode;
	int status;
	wchar_t table[H_GAME][W_GAME]; // Tabela de vizualização
	INT num_tracks;

} api_pipe;

#endif