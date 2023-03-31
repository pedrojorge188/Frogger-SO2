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
       
        _tprintf(L"->");

        WaitForSingleObject(p->mutex, INFINITE);

        if (_tscanf_s(_T("%s %d"), command, 50, &value) == 2) {

            if (wcscmp(command, _T("exit")) == 0) {
                ExitThread(1);
            }
            else if (wcscmp(command, _T("tracks")) == 0) {

                if (value <= 8 && value > 0) {

                    p->gameData->num_tracks = value;
                    _tprintf(_T("[SERVER] Número de estradas alterado! (tracks:%d)\n"),p->gameData->num_tracks);

                    if (ChangeNumTracks(value) == 1) 
                        _tprintf(_T("[GLOBAL] Número de estradas alterado!"));
                    
                }
                else {
                    _tprintf(_T("[SERVER] Valor Inválido\n"));
                }

            }
            else if (wcscmp(command, _T("vspeed")) == 0) {

                p->gameData->vehicle_speed = value;
                _tprintf(_T("[SERVER] Velocidade inícial das viaturas alterado! (vspeed:%d)\n"), p->gameData->vehicle_speed);

                if (ChangeSpeed(value) == 1)
                    _tprintf(_T("[GLOBAL] Velocidade inícial das viaturas alterado!"));
            }
            else if(wcscmp(command, _T("see")) == 0) {

                _tprintf(_T("[SERVER] Número de estradas : %d\n"), p->gameData->num_tracks);
                _tprintf(_T("[SERVER] Velocidade Inicial : %d\n"), p->gameData->vehicle_speed);
                
            }

        }

        ReleaseMutex(p->mutex);

    }

    ExitThread(1);
}

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    if (OpenMutex(MUTEX_ALL_ACCESS, FALSE, SERVER_MUTEX) != NULL) {
        _tprintf(TIMEOUT_MSG); Sleep(TIMEOUT_10_SECONDS);
        return 0;
    }
    
    game gameData;
    thParams structTh = { 0 };

    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];

    gameData = FillRegistryValues();
    
    _tprintf(TEXT("-----------SERVER--------------\n"));
    

    HANDLE serverMutex = CreateMutex(NULL, FALSE, SERVER_MUTEX);

    structTh.mutex = serverMutex;
    structTh.gameData = &gameData;

    hThreads[0] = CreateThread(NULL,0,input_thread,& structTh, 0, &dwIDThreads[0]);

    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);

    CloseHandle(structTh.mutex);

    return 0;
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
}

