#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "operator.h"


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
            else if (wcscmp(command, _T("stoptime")) == 0) {

                if (value <= 0)
                    _tprintf(_T("[SERVER] Valor Inválido!\n"));
                else
                    _tprintf(_T("[SERVER] Valor alterado!\n"));

            }
            else if (wcscmp(command, _T("obstacle")) == 0) {

                _tprintf(_T("[SERVER] Obstaculo colocado!\n"));

            }
            else if (wcscmp(command, _T("invertdir")) == 0) {

                _tprintf(_T("[SERVER] Direção invertida!\n"));

            }

        }

        ReleaseMutex(p->mutex);

    }

    ExitThread(1);
}

int _tmain(int argc, TCHAR* argv[]) {
       
    UNICODE_INITIALIZER();

    if (OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SERVER_SEMAPHORE) == NULL) {
        _tprintf(L"O servidor não está a correr");
        ExitProcess(1);
    }

    thParams structTh = { 0 };
    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];

    _tprintf(TEXT("-----------OPERATOR--------------\n"));

    structTh.mutex = CreateMutex(NULL, FALSE, NULL);

    hThreads[0] = CreateThread(NULL, 0, input_thread, &structTh, 0, &dwIDThreads[0]);

    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);

    CloseHandle(structTh.mutex);

    return 0;
}


void UNICODE_INITIALIZER() {

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}

