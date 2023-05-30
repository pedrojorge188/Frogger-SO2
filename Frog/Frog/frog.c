#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"

OVERLAPPED overlapped ;

void ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}

void WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}

DWORD WINAPI receive_thread(HANDLE p) {

    api receive;
    DWORD bytesAvailable = 0;

    while (1) {


        BOOL pipeHasData = PeekNamedPipe(p, NULL, 0, NULL, &bytesAvailable, NULL);

        if (pipeHasData && bytesAvailable > 0) {

            if (ReadFileEx(p, &receive, sizeof(receive), &overlapped, &ReadCompletionRoutine))
            {

                if (receive.msg == -1) {
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
    }

    ExitThread(1);

}

DWORD WINAPI input_thread(HANDLE p) {

    api send;
    char key;
    TCHAR buf[256];

    int out_flag = 0;

    while (out_flag == 0) {

        key = 0;
        key = getch();

        if (key == 27) {/*esc pressionado*/ key = 1; out_flag = 1; }
        if (key == 72) {/*SETA CIMA pressionado*/ key = 2; }
        if (key == 75) {/*SETA ESQUERDA pressionado*/ key = 3; }
        if (key == 77) {/*SETA DIREITA pressionado*/ key = 4; }


        if (key == 1 || key == 2 || key == 3 || key == 4) {
            send.key = key;
            WriteFileEx(p, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
        }
      

    }

    ExitThread(2);

}

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();
    api initialize_game;
    initialize_game.msg = 2;

    HANDLE hThreads[2];

    if (!WaitNamedPipe(PIPE_NAME, 10000)) {
        _tprintf(L"Server is not running!\n");
        exit(-1);
    }

	HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    WriteFile(hPipe, &initialize_game, sizeof(initialize_game), 0, NULL);

    ZeroMemory(&overlapped, sizeof(overlapped));

    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	_tprintf(TEXT("[FROG] Connection made...\n"));

    hThreads[0] = CreateThread(NULL, 0, input_thread, hPipe, 0, 0);
    hThreads[1] = CreateThread(NULL, 0, receive_thread, hPipe, 0, 0);

    WaitForMultipleObjects(2, hThreads, FALSE, INFINITE);

    return 0;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}