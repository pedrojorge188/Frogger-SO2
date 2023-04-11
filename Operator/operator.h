#ifndef OPERATOR_H
#define OPERATOR_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "..//utils.h"

#define TAM 40
#define MAX_THREADS 3
#define CMD_NOT_FOUND TEXT("COMMAND NOT FOUND !\n")

int out_flag;


void UNICODE_INITIALIZER();
DWORD __stdcall input_thread(LPVOID lpParam);

#endif