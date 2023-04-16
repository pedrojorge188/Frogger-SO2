#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "server.h"


DWORD WINAPI input_thread(LPVOID lpParam) {
    thParams* p = (thParams*)lpParam;

    TCHAR command[50];
    INT value;

    while (1) {
        WaitForSingleObject(p->mutex, INFINITE);

        _tprintf(L"->");

        if (_tscanf_s(_T("%s %d"), command, sizeof(command), &value) == 2) {

            if (wcscmp(command, _T("exit")) == 0) {

                HANDLE shutDown = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVER_SHUTDOWN);
                SetEvent(shutDown);
                CloseHandle(shutDown);

                out_flag = 1;
                ExitThread(1);

            }
            else if (wcscmp(command, _T("tracks")) == 0) {

                if (value <= 8 && value > 0) {

                    _tprintf(_T("[SERVER] Número de estradas inicial alterado! (tracks:%d)\n"), value);

                    if (ChangeNumTracks(value) == 1)
                        _tprintf(_T("[GLOBAL] Número de estradas alterado!"));

                }
                else {
                    _tprintf(_T("[SERVER] Valor Inválido\n"));
                }

            }
            else if (wcscmp(command, _T("vspeed")) == 0) {

                _tprintf(_T("[SERVER] Velocidade inícial das viaturas alterado! (vspeed:%d)\n"), value);

                if (ChangeSpeed(value) == 1)
                    _tprintf(_T("[GLOBAL] Velocidade inícial das viaturas alterado!"));
            }
            else if (wcscmp(command, _T("list")) == 0) {

                _tprintf(_T("[SERVER] Número de estradas Inicial: %d\n"), p->gameData->num_tracks);
                _tprintf(_T("[SERVER] Velocidade Inicial : %d\n"), p->gameData->vehicle_speed);

            }
            else if (wcscmp(command, _T("pause")) == 0) {

                DWORD suspend_count = SuspendThread(p->thIDs[1]);
                if (suspend_count == (DWORD)-1) {
                    _tprintf(L"[SERVER] Erro ao suspender a thread. Código de erro: %d\n", GetLastError());
                    return 1;
                }
                else {
                    _tprintf(L"Jogo Pausado!\n");
                }


            }
            else if (wcscmp(command, _T("resume")) == 0) {

                DWORD resume_count = ResumeThread(p->thIDs[1]); 
                if (resume_count == (DWORD)-1) {
                    _tprintf(L"Erro ao retomar a execução da thread. Código de erro: %d\n", GetLastError());
                    return 1;
                }
                else {
                    _tprintf(L"Jogo Iniciado!\n");
                }

            }
            else if (wcscmp(command, _T("restart")) == 0) {

                FillGameDefaults(p->gameData);
                _tprintf(L"Jogo reiniciado\n");

            }

            ReleaseMutex(p->mutex);
        }

    }

    ExitThread(1);
}

DWORD WINAPI game_manager(LPVOID lpParam) {

    thParams* p = (thParams*)lpParam;
    int pause_game = 0;

    HANDLE mutex = CreateMutex(NULL, FALSE, SHARED_MUTEX);
    if (mutex == NULL) {
        _tprintf(L"Fail to create a mutex!\n");
        CloseHandle(mutex);
        ExitThread(2);
    }

    FillGameDefaults(p->gameData);

    //Código para colocar dentro da dll

    LPVOID hSharedMemory = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        SHARED_MEMORY_SIZE,
        SHARED_MEMORY_NAME
    );


    if (hSharedMemory == NULL) {
        ExitThread(2);
    }

    game* lpSharedMemory = (game*)MapViewOfFile(
        hSharedMemory,
        FILE_MAP_WRITE,
        0,
        0,
        SHARED_MEMORY_SIZE);

    if (lpSharedMemory == NULL) {

        CloseHandle(hSharedMemory);
        ExitThread(2);
    }


    while (out_flag == 0) {

        COORD position = { 2, 3 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;

        Sleep(p->gameData->vehicle_speed * 150);

        WaitForSingleObject(mutex, INFINITE);

        if(pause_game == 0)
            moveCars(p->gameData);

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++) {
                lpSharedMemory->table[i][j] = p->gameData->table[i][j];
            }
        }

        lpSharedMemory->frogs[0] = p->gameData->frogs[0];
        lpSharedMemory->frogs[1] = p->gameData->frogs[1];
        

        if (wcscmp(lpSharedMemory->cmd, _T("dir")) == 0) {

            _tprintf(_T("\n[OPERATOR] Direção invertida!\n"));
            invertOrientation(p->gameData);
            wcscpy_s(lpSharedMemory->cmd, sizeof(_T(" ")), _T(" "));

        }

        else if (wcscmp(lpSharedMemory->cmd, _T("object")) == 0) {

            _tprintf(L"%s", lpSharedMemory->cmd);
            _tprintf(_T("\n[OPERATOR] Obstaculo colocado!\n"));
            setObstacle(p->gameData);
            wcscpy_s(lpSharedMemory->cmd, sizeof(_T(" ")), _T(" "));

        }
        else if (wcscmp(lpSharedMemory->cmd, _T("game")) == 0) {


            if (pause_game == 0) {
                _tprintf(_T("\n[OPERATOR] Jogo pausado!\n"));
                pause_game = 1;
            } else {
                _tprintf(_T("\n[OPERATOR] Jogo retomado!\n"));
                pause_game = 0;
            }

            wcscpy_s(lpSharedMemory->cmd, sizeof(_T(" ")), _T(" "));
        }


        ReleaseMutex(mutex);

    }


    UnmapViewOfFile(lpSharedMemory);
    CloseHandle(hSharedMemory);
    CloseHandle(mutex);
    ExitThread(2);
}

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE verifySemaphore, shutDownEvent;
    DWORD dwWaitResult;

    shutDownEvent = CreateEvent(NULL, TRUE, FALSE, SERVER_SHUTDOWN);

    verifySemaphore = CreateSemaphore(NULL, SERVER_LIMIT_USERS, SERVER_LIMIT_USERS, SERVER_SEMAPHORE);

    if (verifySemaphore == NULL) {
        verifySemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SERVER_SEMAPHORE);
        if (verifySemaphore == NULL) {
            return 1;
        }
    }

    dwWaitResult = WaitForSingleObject(verifySemaphore, 0L);

    if (dwWaitResult != WAIT_OBJECT_0) {
        _tprintf(SERVER_RUNNING_MSG); Sleep(TIMEOUT);
        return -1;
    }


    game gameData;
    thParams structTh = { 0 };

    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];

    gameData = FillRegistryValues();

    HANDLE serverMutex = CreateMutex(NULL, FALSE, NULL);

    structTh.mutex = serverMutex;
    structTh.gameData = &gameData;
    structTh.thIDs = &hThreads;

    hThreads[0] = CreateThread(NULL, 0, input_thread, &structTh, 0, &dwIDThreads[0]);
    hThreads[1] = CreateThread(NULL, 0, game_manager, &structTh, CREATE_SUSPENDED, &dwIDThreads[1]);



    ResumeThread(hThreads[1]);

    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);

    CloseHandle(structTh.mutex);
    CloseHandle(verifySemaphore);
    SetEvent(shutDownEvent);
    CloseHandle(shutDownEvent);

    return 0;
}

void invertOrientation(game *g) {
        
    for (int i = 0; i < g->num_tracks; i++) {
        for (int j = 0; j < g->n_cars_per_track; j++) {
            if (g->cars[i][j].orientation == 1) {
                g->cars[i][j].orientation = 2;
            }
            else {
                g->cars[i][j].orientation = 1;
            }
          
        }
    }

}

int setObstacle(game* g) {

    srand(time(NULL));
    int x;
    int y;


    do {
        x = rand() % W_GAME;
        y = rand() % g->num_tracks;

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++)
                if (g->table[y][x] == 'O')
                    x = 0;

        }

    } while (x == 0 || y == 0);

    g->table[y][x] = 'O';


    return 1;

}

int FillGameDefaults(game* g) {

    srand(time(NULL));
    int direction = 0;

    g->n_cars_per_track = rand() % MAX_VEHICLES;
    if (g->n_cars_per_track == 0)
        g->n_cars_per_track = DEFAULT;

    memset(g->table, ' ', sizeof(g->table));

    //colocar os sapos->META 1

    for (int i = 0; i < MAX_FROGS; i++) {
        g->frogs[i].x = 0;
        g->frogs[i].y = rand() % W_GAME;
        g->frogs[i].points = 0;
        g->table[g->frogs[i].x][g->frogs[i].y] = 'S';
    }


    //Colocar os carros
    for (int i = 0; i < g->num_tracks; i++) {
        direction = rand() % 2;
        for (int j = 0; j < g->n_cars_per_track; j++) {
            g->cars[i][j].orientation = direction;
            g->cars[i][j].x = i + 1;
            g->cars[i][j].y = rand() % W_GAME;

            if (direction == 1)
                g->table[g->cars[i][j].x][g->cars[i][j].y] = '>';
            else
                g->table[g->cars[i][j].x][g->cars[i][j].y] = '<';
        }
    }

    return 1;
}

void moveCars(game* g) {

    for (int i = 0; i < H_GAME; i++) {
        for (int j = 0; j < W_GAME; j++) {
            if (g->table[i][j] != '<' && g->table[i][j] != 'S' && g->table != '>' && g->table != '-' && g->table[i][j] != 'O')
                g->table[i][j] = ' ';
        }
    }

    for (int i = 0; i < g->num_tracks; i++) {
        for (int j = 0; j < g->n_cars_per_track; j++) {
            if (g->table[g->cars[i][j].x][g->cars[i][j].y] != 'O')
                g->table[g->cars[i][j].x][g->cars[i][j].y] = ' ';
            if (g->cars[i][j].orientation == 1) {
                g->cars[i][j].y += 1;
                if (g->cars[i][j].y >= W_GAME) {
                    g->cars[i][j].y -= W_GAME - 2;
                }
            }
            else {
                g->cars[i][j].y -= 1;
                if (g->cars[i][j].y < 1) {
                    g->cars[i][j].y += W_GAME - 2;
                }
            }

            if (g->table[g->cars[i][j].x][g->cars[i][j].y] != 'O') {

                if (g->cars[i]->orientation == 1)
                    g->table[g->cars[i][j].x][g->cars[i][j].y] = '>';
                else
                    g->table[g->cars[i][j].x][g->cars[i][j].y] = '<';

            }

        }

    }

    /*
    _tprintf(L"\n");
    for (int i = 0; i < H_GAME; i++) {
        for (int j = 0; j < W_GAME; j++) {
            _tprintf(L"%c", g->table[i][j]);
        }
        _tprintf(L"\n");
    }
    */
}

int ChangeNumTracks(INT value) {

    HKEY key;
    DWORD dwValue;
    int ret = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_WRITE, &key) == ERROR_SUCCESS) {

        DWORD newValue = (DWORD)value;

        LONG err = RegSetValueEx(key, N_TRACKS_ATT, 0, REG_DWORD, &newValue, sizeof(newValue));

        if (err != ERROR_SUCCESS) {
            _tprintf(_T("[SERVER] ERRO ao atualizar os valores do registry !"));
            ret = 1;
        }


    }

    RegCloseKey(key);
    return ret;

}

int ChangeSpeed(INT value) {

    HKEY key;
    DWORD dwValue;
    int ret = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, KEY_WRITE, &key) == ERROR_SUCCESS) {

        DWORD newValue = (DWORD)value;

        LONG err = RegSetValueEx(key, START_SPEED_ATT, 0, REG_DWORD, &newValue, sizeof(newValue));

        if (err != ERROR_SUCCESS) {
            _tprintf(_T("[SERVER] ERRO ao atualizar os valores do registry !"));
            ret = 1;
        }


    }

    RegCloseKey(key);
    return ret;

}

game FillRegistryValues() {

    game gameData;
    HKEY key, KeySuccess;
    DWORD dwValue;
    WCHAR wcValue[TAM];
    PVOID pvData = wcValue;

    //Trata dos dados das estradas para o registry

    if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_READ, &key) == ERROR_SUCCESS) {

        DWORD size = sizeof(wcValue);
        LONG err = RegGetValue(key, NULL, N_TRACKS_ATT, RRF_RT_ANY, NULL, pvData, &size);

        if (err == ERROR_SUCCESS) {
            DWORD convert = *(DWORD*)pvData;
            gameData.num_tracks = (INT)convert;
        }
        else {
            _tprintf(_T("[SERVER] ERRO ao receber os valores do registry !"));
        }

    }
    else {

        do {
            _tprintf(COUT_TRACKS);
            _tscanf_s(_T("%u"), &dwValue, sizeof(dwValue));
            gameData.num_tracks = (INT)dwValue;

        } while (gameData.num_tracks > (INT)GAME_MAX_TRACKS || gameData.num_tracks < (INT)1);

        if (RegCreateKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_ALL_ACCESS, &KeySuccess) == ERROR_SUCCESS)
                RegSetValueEx(KeySuccess, N_TRACKS_ATT, 0, REG_DWORD, &dwValue, sizeof(dwValue));

            RegCloseKey(KeySuccess);
        }

    }

    RegCloseKey(key);

    //Trata dos dados da velocidade inicial para o registry

    if (RegOpenKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, KEY_READ, &key) == ERROR_SUCCESS) {

        DWORD size = sizeof(wcValue);
        LONG err = RegGetValue(key, NULL, START_SPEED_ATT, RRF_RT_ANY, NULL, pvData, &size);

        if (err == ERROR_SUCCESS) {
            DWORD convert = *(DWORD*)pvData;
            gameData.vehicle_speed = (INT)convert;
        }
        else {
            _tprintf(_T("[SERVER] ERRO ao receber os valores do registry !"));
        }

    }
    else {

        _tprintf(COUT_SPEED);
        _tscanf_s(_T("%u"), &dwValue, sizeof(dwValue));
        gameData.vehicle_speed = (INT)dwValue;

        if (RegCreateKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, KEY_ALL_ACCESS, &KeySuccess) == ERROR_SUCCESS)
                RegSetValueEx(KeySuccess, START_SPEED_ATT, 0, REG_DWORD, &dwValue, sizeof(dwValue));

            RegCloseKey(KeySuccess);
        }

    }

    RegCloseKey(key);

    return gameData;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    _tprintf(TEXT("-----------SERVER--------------\n"));
}