#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"
#include "..//utils.h"

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE verifySemaphore, hPipe;
    DWORD dwWaitResult;


    if (OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SERVER_SEMAPHORE) == NULL) {
        _tprintf(L"O servidor não está a correr");
        ExitProcess(1);
    }


    verifySemaphore = CreateSemaphore(NULL, MAX_FROGS, MAX_FROGS, FROG_SEMAPHORE);

    if (verifySemaphore == NULL) {

        verifySemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, FROG_SEMAPHORE);

        if (verifySemaphore == NULL) {
            _tprintf(_T("Erro ao criar semáforo do frog\n"));
            return 1;
        }

    }

    dwWaitResult = WaitForSingleObject(verifySemaphore, 0L);
    if (dwWaitResult != WAIT_OBJECT_0) {
        _tprintf(FROG_RUNNING_MSG);
        Sleep(TIMEOUT);
        return -1;
    }

    if (!WaitNamedPipe(PIPE_NAME, 10000)) {
        exit(-1);
    }


    hPipe = CreateFile(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
        exit(-1);
    }

    _tprintf(TEXT("[FROG] I'm Connected to server!...\n"));

    api_pipe receive;


    while (1) {
        //le as mensagens enviadas pelo servidor
        DWORD ret = ReadFile(hPipe, &receive, sizeof(receive), 0, NULL);

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

    CloseHandle(hPipe);
    CloseHandle(verifySemaphore);

    return 0;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}