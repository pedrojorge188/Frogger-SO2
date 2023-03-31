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

        }

        ReleaseMutex(p->mutex);

    }

    ExitThread(1);
}

int _tmain(int argc, TCHAR* argv[]) {

    thParams structTh = { 0 };

    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];

    UNICODE_INITIALIZER();

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

