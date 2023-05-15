#ifndef FROG_H
#define FROG_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define TAM 40

#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define TIMEOUT 10000
#define FROG_RUNNING_MSG TEXT("Já existe dois programas frog em atividade!")
#define FROG_SEMAPHORE TEXT("FrogInstances")

#define N_TRACKS TEXT("FroggerGame\NumberTracks")

#define START_SPEED TEXT("FroggerGame\StartSpeed")

#define MAX_FROGS 2

typedef struct thParams {
    HANDLE hPipe;
}thParams;

void UNICODE_INITIALIZER();

#endif