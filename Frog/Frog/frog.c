#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE verifySemaphore;//verifica sapos "abertos"
    DWORD dwWaitResult;

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
        _tprintf(FROG_RUNNING_MSG); Sleep(TIMEOUT);
        return -1;
    }

    _tprintf(_T("Entrou\n"));
    Sleep(10000);
    

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