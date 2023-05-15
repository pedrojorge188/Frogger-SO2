#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"
#include "..\\..\\utils.h"

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE verifySemaphore,hPipe;
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