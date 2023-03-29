#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "server.h"

game FillRegistryValues(game gameData) {

    HKEY key, KeySuccess;
    DWORD dwValue;
    WCHAR wcValue[TAM];
    PVOID pvData = wcValue;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_READ, &key) == ERROR_SUCCESS) {
           
        DWORD size = sizeof(wcValue);
        LONG err = RegGetValue(key, NULL, N_TRACKS_ATT, RRF_RT_ANY, NULL, pvData, &size);

        if (err == ERROR_SUCCESS) {
            DWORD convert = *(DWORD*)pvData;
            gameData.num_tracks = (INT)convert;
        }else {
            _tprintf(_T("[SERVER] ERRO ao receber os valores do registry !"));
        }

    }else {
        
        do {
            _tprintf(COUT_TRACKS);
            _tscanf_s(_T("%u"), &dwValue, sizeof(dwValue));
            gameData.num_tracks = (INT)dwValue;

        } while (gameData.num_tracks > (INT) GAME_MAX_TRACKS || gameData.num_tracks < (INT) 1);

        if (RegCreateKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, NULL, REG_OPTION_VOLATILE,KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_ALL_ACCESS, &KeySuccess) == ERROR_SUCCESS) 
                RegSetValueEx(KeySuccess, N_TRACKS_ATT, 0, REG_DWORD, &dwValue, sizeof(dwValue));

            RegCloseKey(KeySuccess);
        }

    }

    RegCloseKey(key);
    return gameData;
}

int _tmain(int argc, TCHAR* argv[]) {

    unicodeInitializer();
    game gameData;
    gameData = fillGameStruct();
    gameData = FillRegistryValues(gameData);



    return 0;
}

game fillGameStruct() {
    game g;
    g.num_tracks = 0;
    g.vehicle_speed = 0;
    return g;
}

void unicodeInitializer() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}