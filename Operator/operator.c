#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "operator.h"
#include <stdlib.h>
#include <stdbool.h>

typedef game * (*RECEIVE_GAME)(void);
typedef int (*WRITE_CMD)(TCHAR *);

DWORD WINAPI server_info(LPVOID lpParam) {

    COORD pos = { 0 , 18 };
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    while (out_flag == 0) {

        if (WaitForSingleObject(OpenEventW(EVENT_ALL_ACCESS, FALSE, SERVER_SHUTDOWN), INFINITE) == WAIT_OBJECT_0) {

            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

            _tprintf(L"O servidor encerrou a sess�o !");

            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
            ExitProcess(1);
        }
    }

    ExitThread(3);
}

DWORD WINAPI input_thread(LPVOID lpParam) {

    DataTh* data = (DataTh*)lpParam;

    TCHAR command[100] = L" ";
    INT value;
    COORD pos = { 0 , 18 };

    HINSTANCE hinstDLL = LoadLibrary(TEXT("sharedMemoryInterator.dll"));

    while (wcscmp(command, _T("exit")) != 0) {
           

         
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);

            _tprintf(L"->");

            _tscanf_s(_T("%s %d"), command, 50, &value);

            WaitForSingleObject(data->hWrite, INFINITE);
            WaitForSingleObject(data->trinco, INFINITE);
           
            if (hinstDLL != NULL) {
                WRITE_CMD writeCmd = (WRITE_CMD)GetProcAddress(hinstDLL, "write_cmds_to_shared_memory");
                writeCmd(command);
            }
   
            ReleaseMutex(data->trinco);
            ReleaseSemaphore(data->hRead, 1, NULL);
    }

    FreeLibrary(hinstDLL);
    exit(1);
    ExitThread(1);
}

DWORD WINAPI game_informations(LPVOID lpParam) {

    /*Esta thread � responsavel por mostrar o estado atual do jogo partilhado pelo server*/

    game g;
    game* receiver = { 0 };
    
    HINSTANCE hinstDLL = LoadLibrary(TEXT("sharedMemoryInterator.dll"));

    HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, SHARED_MUTEX);
    if (mutex == NULL) {
        _tprintf(L"Fail to open mutex!\n");
        CloseHandle(mutex);
        ExitThread(2);
    }
    
    int ret = 0;
  

    while (out_flag == 0) {
        
        if (hinstDLL != NULL) {
            RECEIVE_GAME copyGame = (RECEIVE_GAME)GetProcAddress(hinstDLL, "get_game_from_shared_memory");
            receiver = copyGame();
        }

        WaitForSingleObject(mutex,INFINITE);
        

        COORD position = { 5 , 2 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++) {

                g.table[i][j] = receiver->table[i][j];
    
            }
        }

        g.frogs[0] = receiver->frogs[0]; g.frogs[1] = receiver->frogs[1];

        for (int i = H_GAME-1; i >= -1; i--) {
            for (int j = W_GAME-1; j >= 0; j--) {


                wchar_t c = g.table[i][j];

                if (i == receiver->num_tracks + 1 || i == receiver->num_tracks + 2)

                    c = L'_';

                else if (i == 0 && g.table[i][j] != L'S' || i == -1)

                    c = L'_';

                if ((j == 0 || j == W_GAME-1) && i < receiver->num_tracks + 2)

                    c = L'|';

                WriteConsoleOutputCharacterW(console, &c, 1, position, &written);
                position.X++;

            }
            position.Y++;
            position.X = 5;
        }



        wchar_t myString[20] = L"Pontos Sapo 1: ";
        wchar_t myString2[20] = L"Pontos Sapo 2: ";
        int sp_1 = g.frogs[0].points;
        int sp_2 = g.frogs[1].points;

        swprintf_s(myString + wcslen(myString), 20 - wcslen(myString), L"%d", sp_1);
        swprintf_s(myString2 + wcslen(myString2), 20 - wcslen(myString2), L"%d", sp_2);
       
        WriteConsoleOutputCharacterW(console, myString, wcslen(myString), (COORD){0, 15}, &written);
        WriteConsoleOutputCharacterW(console, myString2, wcslen(myString2), (COORD) { wcslen(myString2)+5 , 15 }, & written);

        ReleaseMutex(mutex);
        
    }

    FreeLibrary(hinstDLL);
    CloseHandle(mutex);
    
    ExitThread(2);
}

int _tmain(int argc, TCHAR* argv[]) {
       
    UNICODE_INITIALIZER();
    
    if (OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SERVER_SEMAPHORE) == NULL) {
        _tprintf(L"O servidor n�o est� a correr");
        return -1;
    }
    

    DataTh data;

    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];

    data.hRead = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, READ_SEMAPHORE);
    data.hWrite = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, WRITE_SEMAPHORE);
    data.trinco = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_COMMAND_ACCESS);


    if (data.hRead == NULL || data.hWrite == NULL || data.trinco == NULL) {
        _tprintf(L"[ERROR] Opening handles\n");
        return -1;
    }

    hThreads[0] = CreateThread(NULL, 0, input_thread, &data, 0, &dwIDThreads[0]);
    hThreads[1] = CreateThread(NULL, 0, game_informations, NULL, 0, &dwIDThreads[1]);
    hThreads[2] = CreateThread(NULL, 0, server_info, NULL, 0, &dwIDThreads[0]);

    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);

    CloseHandle(data.hRead);
    CloseHandle(data.hWrite);
    CloseHandle(data.trinco);

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

