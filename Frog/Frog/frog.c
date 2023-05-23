#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"

OVERLAPPED overlapped ;

void CALLBACK ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped){}

void CALLBACK WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped){}

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();
    api receive = { 0 };

    if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(L"Server is not running!\n");
        exit(-1);
    }

	HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Liguei-me...\n"));

    while (1) {


        if (ReadFileEx( hPipe,&receive, sizeof(receive),&overlapped, &ReadCompletionRoutine))
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

    return 0;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}