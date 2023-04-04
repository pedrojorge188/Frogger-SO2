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

        COORD position = { 0, 18 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
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

                    _tprintf(_T("[SERVER] N�mero de estradas inicial alterado! (tracks:%d)\n"),value);

                    if (ChangeNumTracks(value) == 1) 
                        _tprintf(_T("[GLOBAL] N�mero de estradas alterado!"));
                    
                }
                else {
                    _tprintf(_T("[SERVER] Valor Inv�lido\n"));
                }

            }
            else if (wcscmp(command, _T("vspeed")) == 0) {

                _tprintf(_T("[SERVER] Velocidade in�cial das viaturas alterado! (vspeed:%d)\n"), value);

                if (ChangeSpeed(value) == 1)
                    _tprintf(_T("[GLOBAL] Velocidade in�cial das viaturas alterado!"));
            }
            else if(wcscmp(command, _T("list")) == 0) {

                _tprintf(_T("[SERVER] N�mero de estradas Inicial: %d\n"), p->gameData->num_tracks);
                _tprintf(_T("[SERVER] Velocidade Inicial : %d\n"), p->gameData->vehicle_speed);
                
            }
            else if (wcscmp(command, _T("pause")) == 0) {

                _tprintf(L"Jogo Pausado!\n");

            }
            else if (wcscmp(command, _T("resume")) == 0) {
                 
                _tprintf(L"Jogo Retornado\n");

            }
            else if (wcscmp(command, _T("restart")) == 0) {

                _tprintf(L"Jogo reiniciado\n");

            }

            ReleaseMutex(p->mutex);
        }

    }

    ExitThread(1);
}

DWORD WINAPI game_manager(LPVOID lpParam) {

    thParams* p = (thParams*)lpParam;

    FillGameDefaults(p->gameData);

    while (out_flag == 0) {

        moveCars(p->gameData);

        // O C�DIGO SEGUINTE � PARA SER IMPLEMENTADO NO OPERATOR
        COORD position = { 2, 3 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++) {
                WCHAR c = p->gameData->table[i][j];
                WriteConsoleOutputCharacterW(console, &c, 1, position, &written);
                position.X++;
            }
            position.Y++;
            position.X = 0;
        }

        Sleep(p->gameData->vehicle_speed * 150);

    }

    ExitThread(2);
}

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE verifySemaphore, shutDownEvent;
    DWORD dwWaitResult;

    shutDownEvent = CreateEvent(NULL, TRUE, FALSE, SERVER_SHUTDOWN);

    verifySemaphore = CreateSemaphore(NULL, SERVER_LIMIT_USERS, SERVER_LIMIT_USERS, SERVER_SEMAPHORE);

    if (verifySemaphore == NULL){
        verifySemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SERVER_SEMAPHORE);
        if (verifySemaphore == NULL){
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

    hThreads[0] = CreateThread(NULL,0,input_thread, &structTh, 0, &dwIDThreads[0]);
    hThreads[1] = CreateThread(NULL, 0,game_manager, &structTh, CREATE_SUSPENDED, &dwIDThreads[0]);
    
    ResumeThread(hThreads[1]); 

    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);

    CloseHandle(structTh.mutex);
    CloseHandle(verifySemaphore);
    SetEvent(shutDownEvent);
    CloseHandle(shutDownEvent);

    return 0;
}

int FillGameDefaults(game * g){

    srand(time(NULL));
    int direction = 0;

    g->n_cars_per_track = rand()%MAX_VEHICLES;
    if (g->n_cars_per_track == 0)
        g->n_cars_per_track = DEFAULT;

    memset(g->table, ' ', sizeof(g->table));

    //colocar os sapos->META 1
    
    for (int i = 0; i < MAX_FROGS; i++) {
        g->frogs[i].x = 0;
        g->frogs[i].y = rand() % W_GAME;
        g->table[g->frogs[i].x][ g->frogs[i].y] = 'S';
    }
   

    //Colocar os carros
    for (int i = 0; i < g->num_tracks; i++) {
        direction = rand() % 2;
        for (int j = 0; j < g->n_cars_per_track; j++) {
            g->cars[i][j].orientation = direction;
            g->cars[i][j].x = i+1;
            g->cars[i][j].y = rand() % W_GAME;
            g->table[g->cars[i][j].x][g->cars[i][j].y] = 'V';
        }
    }

    return 1;
}

void moveCars(game* g) {

    for (int i = 0; i < H_GAME; i++) {
        for (int j = 0; j < W_GAME; j++) {
            if (g->table[i][j] != 'V' && g->table[i][j] != 'S')
                g->table[i][j] = ' ';
        }
    }

    for (int i = 0; i < g->num_tracks; i++) {
        for (int j = 0; j < g->n_cars_per_track; j++) {
            g->table[g->cars[i][j].x][g->cars[i][j].y] = ' ';
            if (g->cars[i][j].orientation == 1) {
                g->cars[i][j].y += 1;
                if (g->cars[i][j].y >= W_GAME) {
                    g->cars[i][j].y -= W_GAME-2;
                }
            }
            else {
                g->cars[i][j].y -= 1;
                if (g->cars[i][j].y < 1) {
                    g->cars[i][j].y += W_GAME-2;
                }
            }
            g->table[g->cars[i][j].x][g->cars[i][j].y] = 'V';
        }

    }
     
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

