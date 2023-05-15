#pragma once

#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define SHARED_MEMORY_CMDS TEXT("CMD_MEMORY")
#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define SERVER_SHUTDOWN TEXT("ServerExit")
#define SHARED_MEMORY_NAME TEXT("MEM_WITH_DATA")
#define SHARED_MUTEX TEXT("SHARED_MUTEX")
#define WRITE_SEMAPHORE  TEXT("SEMAFORO_ESCRITA")
#define READ_SEMAPHORE  TEXT("SEMAFORO_LEITURA")
#define MUTEX_COMMAND_ACCESS TEXT("MUTEX_COMANDOS")
#define BUFFER_SIZE 50

#define MAX_FROGS 2
#define W_GAME 20 //largura da area de jogo (COLUNAS)
#define H_GAME 10 //altura da area de jogo (LINHAS)
#define MAX_VEHICLES 8


typedef struct frog {
	int x, y;
	int points; // se -1 perdeu o jogo e � desconectado
}frog;

typedef struct vehicles {
	int x, y;
	int orientation; // 1-> direita para a esquerda, 2-> esquerda para a direita
} vehicles;

typedef struct bufferCircular {
	TCHAR cmd[100];
}bufferCircular;

typedef struct buffer {
	int pRead;
	int pWrite;
	bufferCircular buffer[BUFFER_SIZE];
}buffer;

 typedef struct game {

	frog frogs[2];			//Sapos (Clientes)
	vehicles cars[8][8]; // veiculos[pos na faxa][o id do carro]
	int mode;				// 1-> Individual 2-> Competi��o
	INT num_tracks;			// Numero de estradas
	INT vehicle_speed;	   // Velocidade dos carros
	int track_speed[8];
	int n_cars_per_track; // Random de numero de carros por track
	wchar_t table[H_GAME][W_GAME]; // Tabela de vizualiza��o

}game;

