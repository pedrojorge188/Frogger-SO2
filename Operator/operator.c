#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "operator.h"
#include <stdlib.h>
#include <stdbool.h>


DWORD WINAPI server_info(LPVOID lpParam) {

    COORD pos = { 0 , 18 };
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    while (out_flag == 0) {

        if (WaitForSingleObject(OpenEventW(EVENT_ALL_ACCESS, FALSE, SERVER_SHUTDOWN), INFINITE) == WAIT_OBJECT_0) {

            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

            _tprintf(L"O servidor encerrou a sessão !");

            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
            ExitProcess(1);
        }
    }
  

    ExitThread(3);
}

DWORD WINAPI input_thread(LPVOID lpParam) {

    TCHAR command[50];
    INT value;

    COORD pos = { 0 , 18 };

    while (1) {
           
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
            _tprintf(L"->");

            _tscanf_s(_T("%s %d"), command, 50, &value);

            if (wcscmp(command, _T("exit")) == 0) {

                SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
                out_flag = 1;
                exit(1);
            }
            else if (wcscmp(command, _T("stoptime")) == 0) {

                _tprintf(_T("[SERVER] ´Tempo Parado!\n"));

            }
            else if (wcscmp(command, _T("obstacle")) == 0) {

                _tprintf(_T("[SERVER] Obstaculo colocado!\n"));

            }
            else if (wcscmp(command, _T("invertdir")) == 0) {

                _tprintf(_T("[SERVER] Direção invertida!\n"));

            }
     

    }

    ExitThread(1);
}

DWORD WINAPI game_informations(LPVOID lpParam) {
    
    COORD position = { 0, 1 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    shared_api sd;

    HANDLE hFileShared = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);

    if (hFileShared == NULL)
    {
        _tprintf(L"erro!");
        ExitThread(2);
    }

    LPVOID pBuf  = MapViewOfFile(
        hFileShared, 
        FILE_MAP_ALL_ACCESS,  
        0,
        0,
        SHARED_MEMORY_SIZE);

    if (pBuf == NULL)
    {
        CloseHandle(hFileShared);
        ExitThread(2);
    }

    
    while (out_flag == 0) {
        

        COORD position = { 2, 3 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;
            
        CopyMemory(&sd, pBuf, sizeof(sd));

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++) {
                wchar_t c = sd.table[i][j];
                WriteConsoleOutputCharacterW(console, &c, 1, position, &written);
                position.X++;
            }
            position.Y++;
            position.X = 0;
        }

 
    }

    UnmapViewOfFile(pBuf);

    CloseHandle(hFileShared);
    
    ExitThread(2);
}

int _tmain(int argc, TCHAR* argv[]) {
       
    UNICODE_INITIALIZER();


    if (OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SERVER_SEMAPHORE) == NULL) {
        _tprintf(L"O servidor não está a correr");
        ExitProcess(1);
    }

    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];


    hThreads[0] = CreateThread(NULL, 0, input_thread, NULL, 0, &dwIDThreads[0]);
    hThreads[1] = CreateThread(NULL, 0, game_informations, NULL, 0, &dwIDThreads[1]);
    hThreads[2] = CreateThread(NULL, 0, server_info, NULL, 0, &dwIDThreads[0]);

    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);


    return 0;
   
}


void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

COORD c = { 0,0 };
SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);

_tprintf(TEXT("-----------OPERATOR--------------\n"));
}

