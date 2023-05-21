#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"
#include "..//utils.h"


DWORD WINAPI receive_game(HANDLE p) {


    INT exit_state = 0;
    api_pipe receive;
    DWORD data_received;

    while (exit_state == 0) {

        DWORD ret = ReadFile(p, &receive, sizeof(receive), &data_received, NULL);

        if (data_received > 0) {
              
            if (receive.status == -1) {
                _tprintf(L"[SERVER] Game already running in another mode of game!");
                Sleep(5000);
                ExitThread(1);
            }

            COORD position = { 5 , 2 };
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD written;

            for (int i = H_GAME - 1; i >= -1; i--) {
                for (int j = W_GAME - 1; j >= 0; j--) {


                    wchar_t c = receive.table[i][j];

                    if (i == receive.num_tracks + 1 || i == receive.num_tracks + 2)

                        c = L'_';

                    else if (i == 0 && receive.table[i][j] != L'S' || i == -1)

                        c = L'_';

                    if ((j == 0 || j == W_GAME - 1) && i < receive.num_tracks + 2)

                        c = L'|';

                    WriteConsoleOutputCharacterW(console, &c, 1, position, &written);
                    position.X++;

                }
                position.Y++;
                position.X = 5;
            }

        }
    
    }

    ExitThread(1);

}

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE  hPipe;
    thParams tparam;
    HANDLE Threads[N_THREADS];
    DWORD dwWaitResult;



    if (!WaitNamedPipe(PIPE_NAME, 1000)) {
        _tprintf(L"O servidor não está disponivel");
        exit(-1);
    }

    hPipe = CreateFile(PIPE_NAME,GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
        exit(-1);
    }
    else {
        _tprintf(TEXT("[FROG] I'm Connected to server!...\n"));
    }

    api_pipe init;

    do {
        
        init.mode = 2;
        DWORD ret = WriteFile(hPipe, &init, sizeof(init), 0, NULL);

    } while (init.mode == 0);

    Threads[0] = CreateThread(NULL, 0, receive_game, hPipe, 0, NULL);

    _tprintf(L"GAME MODE (%d)", init.mode);

    WaitForMultipleObjects(N_THREADS, &Threads, TRUE, INFINITE);

    CloseHandle(hPipe);

    return 0;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}