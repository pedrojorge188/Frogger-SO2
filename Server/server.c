#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "server.h"


game sysRegistry(game gameData) {

    HKEY key, KeySuccess;
    DWORD value;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {

        _tprintf(TEXT("Retirar dados do numero de estradas\n"));

    }else {

        do {
            _tprintf(_T("[SERVER] Insira o número de estradas (1-8) "));
            _tscanf_s(_T("%u"), &value, sizeof(value));
            gameData.num_tracks = (INT)value;

        } while (gameData.num_tracks > GAME_MAX_TRACKS || gameData.num_tracks < 1);

        if (RegCreateKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, NULL, REG_OPTION_VOLATILE,KEY_ALL_ACCESS, NULL, &key, NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_ALL_ACCESS, &KeySuccess) == ERROR_SUCCESS) 
                RegSetValueEx(KeySuccess, N_TRACKS_ATT, 0, REG_DWORD, &value, sizeof(value));

            RegCloseKey(KeySuccess);
        }

    }

    RegCloseKey(key);

    return gameData;
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    game gameData;
    gameData.num_tracks = 0;
    gameData.vehicle_speed = 0;

    gameData = sysRegistry(gameData);
    return 0;
}