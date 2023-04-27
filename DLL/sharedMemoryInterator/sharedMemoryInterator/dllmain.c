// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"
#include "DLL.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


__declspec(dllexport) void  initialize_game_shared_memory() {
    
    LPVOID lp = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(game),
        SHARED_MEMORY_NAME
    );

}

__declspec(dllexport) int copy_game_to_sharedMemory(game * g) {

    HANDLE hFileShared = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);

    if (hFileShared == NULL)
        return 0;
 

    game* pBuf = (game*)MapViewOfFile(
        hFileShared,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(game));

    if (pBuf == NULL)
        return 0;

    CopyMemory(pBuf, g, sizeof(game));

    CloseHandle(hFileShared);
    UnmapViewOfFile(pBuf);

}

__declspec(dllexport) game * get_game_from_shared_memory() {


    HANDLE hFileShared = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);

    if (hFileShared == NULL)
        return NULL;


    game* pBuf = (game*)MapViewOfFile(
        hFileShared,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(game));

    if (pBuf == NULL)
        return NULL;

    return pBuf;
    CloseHandle(hFileShared);
    UnmapViewOfFile(pBuf);
}

__declspec(dllexport) void initialize_cmds_shared_memory() {


    LPVOID cmd_shared_list = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(buffer),
        SHARED_MEMORY_CMDS
    );

    buffer* memParser = (buffer*)MapViewOfFile(
        cmd_shared_list,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(buffer));
    
    memParser->pRead = 0;
    memParser->pWrite = 0;

    UnmapViewOfFile(memParser);

}

__declspec(dllexport) int read_cmds_from_shared_memory() {


    HANDLE hFileShared = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_CMDS);

    int ret_status = 0;

    if (hFileShared == NULL)
        return 0;


    buffer* memParser = (buffer*)MapViewOfFile(
        hFileShared,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(buffer));

    if (memParser == NULL) {
        return 0;
    }

    if (wcscmp(memParser->buffer[memParser->pRead].cmd, _T("dir")) == 0) {

        ret_status = 1;

    }
    else if (wcscmp(memParser->buffer[memParser->pRead].cmd, _T("object")) == 0) {

        ret_status = 2;
    }
    else if (wcscmp(memParser->buffer[memParser->pRead].cmd, _T("stopcars")) == 0) {

        ret_status = 3;
        
    }
    else if (wcscmp(memParser->buffer[memParser->pRead].cmd, _T("resume")) == 0) {

        ret_status = 4;

    }

    memParser->pRead++;
    if (memParser->pRead == 50)
        memParser->pRead = 0;

    return ret_status;

    CloseHandle(hFileShared);
    UnmapViewOfFile(memParser);
}

__declspec(dllexport) int write_cmds_to_shared_memory(TCHAR * cmd) {


    HANDLE hFileShared = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_CMDS);

    if (hFileShared == NULL)
        return 0;


    buffer* memParser = (buffer*)MapViewOfFile(
        hFileShared,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(buffer));

    if (memParser == NULL) {
        return 0;
    }

    CopyMemory(memParser->buffer[memParser->pWrite].cmd, cmd, 100);

    memParser->pWrite++;
    if (memParser->pWrite == BUFFER_SIZE)
        memParser->pWrite = 0;
    

    CloseHandle(hFileShared);
    UnmapViewOfFile(memParser);
}

