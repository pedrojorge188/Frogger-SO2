#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define SERVER_SHUTDOWN TEXT("ServerExit")
#define SHARED_MEMORY_NAME TEXT("MEM_WITH_DATA")
#define SHARED_MUTEX TEXT("SHARED_MUTEX")
#define SHARED_MEMORY_SIZE 3000

#define W_GAME 20 //largura da area de jogo (COLUNAS)
#define H_GAME 10 //altura da area de jogo (LINHAS)


typedef struct shared_api {
	wchar_t table[20][10]; // Tabela de vizualização para debug
	TCHAR cmd[100];
	HANDLE mutex;
}shared_api;

#endif