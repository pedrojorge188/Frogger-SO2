#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "frog.h"

int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    _tprintf(_T("Entrou\n"));
    Sleep(10000);


    return 0;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
}