#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "server.h"

typedef void (*INIT_GAME_MEMORY)(void);
typedef int (*COPY_GAME_STATUS)(game*);
typedef void (*INIT_CMDS_MEMORY)(void);
typedef int (*READ_CMDS_FROM_SHARED_MEMORY)(void);


DWORD WINAPI client_manager(LPVOID lpParam) {

    api_pipe msg, start_info;
    thParams* p = (thParams*)lpParam;

    int id = connected_clients-1;
  
    HANDLE mutex = CreateMutex(NULL, FALSE, SHARED_MUTEX);
    if (mutex == NULL) {
        _tprintf(L"[ERROR] connect_clients thread\n");
        CloseHandle(mutex);
        ExitThread(2);
    }


    if (id >= 1 && p->gameData->mode == 1) {

        msg.status = -1;
        DWORD ret = WriteFile(p->pipe[id].hPipe, &msg, sizeof(msg), 0, NULL);

        p->pipe[id].active = FALSE;
        SetEvent(p->pipe[id].overlap.hEvent);
        ExitThread(5);

    }


    setFrog(p->gameData, id);
    _tprintf(L"[SERVER] Frog (%d) Connected \n", id + 1);
   

    WaitForSingleObject(mutex, INFINITE);

        DWORD ret = ReadFile(p->pipe[id].hPipe, &start_info, sizeof(start_info), 0, NULL);

        if (start_info.mode == 1) {
            _tprintf(L"[GAME] Frog (%d) Start Game in gameMode1\n",id + 1);
            p->gameData->mode = 1;
        }
        else if (start_info.mode == 2) {
            _tprintf(L"[GAME] Frog (%d) Start Game in gameMode2\n",id + 1);
            p->gameData->mode = 2;
        }

    ReleaseMutex(mutex);

    while (out_flag == 0)  {


    }

    CloseHandle(mutex);
    ExitThread(5);
   
}


DWORD WINAPI connect_clients(LPVOID lpParam) {

    thParams* p = (thParams*)lpParam;

    while (out_flag == 0) {
            
            DWORD offset = WaitForMultipleObjects(N_CLIENTS, p->hEvents, FALSE, INFINITE);
            int i = offset - WAIT_OBJECT_0; // devolve o indice da instancia do named pipe que está ativa, aqui sabemos em que indice o cliente se ligou

            // se é um indice válido ...
            if (i >= 0 && i < N_CLIENTS) {

                DWORD nBytes;

                if (GetOverlappedResult(p->pipe[i].hPipe,&p->pipe[i].overlap, &nBytes, FALSE)) {

                    ResetEvent(p->hEvents[i]);
                   
                    
                        p->pipe[i].active = TRUE;
                        p->pipe[i].id = connected_clients;
                        connected_clients++;
                        CreateThread(NULL, 0, client_manager, p, 0, NULL);

                }
            }
 
    }


    ExitThread(4);
 }

DWORD WINAPI move_cars(LPVOID lpParam){

    moveParam* p = (moveParam*)lpParam;

    while (out_flag == 0) {

        Sleep(p->gameData->track_speed[p->track] * 200);

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++) {
                if (p->gameData->table[i][j] != '<' && p->gameData->table[i][j] != 'S' && p->gameData->table[i][j] != '>' && p->gameData->table[i][j] != '-' && p->gameData->table[i][j] != 'O')
                    p->gameData->table[i][j] = ' ';
            }
        }

        for (int j = 0; j < p->gameData->n_cars_per_track; j++) {
            if (p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] != 'O')
                p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] = ' ';

            if (p->gameData->cars[p->track][j].orientation == 1) {
                if (p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y + 1] != 'O')
                    p->gameData->cars[p->track][j].y += 1;
                else
                    p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] = '>';
                if (p->gameData->cars[p->track][j].y >= W_GAME) {
                    p->gameData->cars[p->track][j].y -= W_GAME - 2;
                }
            }
            else {
                if (p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y - 1] != 'O')
                    p->gameData->cars[p->track][j].y -= 1;
                else
                    p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] = '<';

                if (p->gameData->cars[p->track][j].y < 1) {
                    p->gameData->cars[p->track][j].y += W_GAME - 2;
                }
            }

            if (p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] != 'O') {

                if (p->gameData->cars[p->track][j].orientation == 1)
                    p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] = '<';
                else
                    p->gameData->table[p->gameData->cars[p->track][j].x][p->gameData->cars[p->track][j].y] = '>';

            }
        }


    }

    ExitThread(3);

}

DWORD WINAPI cmd_receiver(LPVOID lpParam) {

    thParams* p = (thParams*)lpParam;
    HINSTANCE hinstDLL = LoadLibrary(TEXT("sharedMemoryInterator.dll"));

    int cmd_status = 0;

    /*
        cmd_status = 1 -> invertDirection
        cmd_status = 2 -> set object
        cmd_status = 3 -> stopcars
        cmd_status = 4 -> resume car movement
    */


    while (out_flag == 0) {

        WaitForSingleObject(p->hRead, INFINITE);
        WaitForSingleObject(p->hBlock, INFINITE);

        if (hinstDLL != NULL) {
            READ_CMDS_FROM_SHARED_MEMORY cmd_read = (READ_CMDS_FROM_SHARED_MEMORY)GetProcAddress(hinstDLL, "read_cmds_from_shared_memory");
            cmd_status = cmd_read();
        }
        switch (cmd_status)
        {
        case 0:
            break;
        case 1:

            invertOrientation(p->gameData);
            break;
        case 2:

            setObstacle(p->gameData);
            break;

        case 3:

            for (int i = 0; i < p->gameData->num_tracks; i++) {
                if (SuspendThread(p->move_threads[i]) == (DWORD)-1) {
                    return 1;
                }

            }
            break;

        case 4:

            for (int i = 0; i < p->gameData->num_tracks; i++) {
                if (ResumeThread(p->move_threads[i]) == (DWORD)-1) {
                    return 1;
                }

            }

            break;
        }

        ReleaseMutex(p->hBlock);
        ReleaseSemaphore(p->hWrite, 1, NULL);


    }

    FreeLibrary(hinstDLL);
    ExitThread(3);
}

DWORD WINAPI game_manager(LPVOID lpParam) {

    thParams* p = (thParams*)lpParam;
    api_pipe send;
    DWORD n;

    HANDLE mutex = CreateMutex(NULL, FALSE, SHARED_MUTEX);
    if (mutex == NULL) {
        _tprintf(L"[ERROR] game_manager thread\n");
        CloseHandle(mutex);
        ExitThread(2);
    }

    HINSTANCE hinstDLL = LoadLibrary(TEXT("sharedMemoryInterator.dll"));

    while (out_flag == 0) {

        COORD position = { 2, 3 };
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;

        WaitForSingleObject(mutex, INFINITE);

            if (hinstDLL != NULL) {
                COPY_GAME_STATUS copyGame = (COPY_GAME_STATUS)GetProcAddress(hinstDLL, "copy_game_to_sharedMemory");
                copyGame(p->gameData);
            }

            for (int i = 0; i < N_CLIENTS; i++) {
                

                if ( p->pipe[i].active) {
                    
                    for (int i = 0; i < H_GAME; i++) {
                        for (int j = 0; j < W_GAME; j++) {

                            send.table[i][j] = p->gameData->table[i][j];
                            send.num_tracks = p->gameData->num_tracks;

                            ReleaseMutex(mutex);
                        }
                    }

                    WriteFile(p->pipe[i].hPipe, &send, sizeof(send), &n, NULL);
               
                }

		    }


        ReleaseMutex(mutex);

    }

    for (int i = 0; i < N_CLIENTS; i++)
        SetEvent(p->hEvents[i]);

    FreeLibrary(hinstDLL);
    CloseHandle(mutex);
    ExitThread(2);
}

DWORD WINAPI input_thread(LPVOID lpParam) {
    thParams* p = (thParams*)lpParam;

    TCHAR command[50];
    INT value;

    while (1) {

        _tprintf(L"->");

        if (_tscanf_s(_T("%s %d"), command, sizeof(command), &value) == 2) {

            if (wcscmp(command, _T("exit")) == 0) {

                HANDLE shutDown = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVER_SHUTDOWN);
                SetEvent(shutDown);
                CloseHandle(shutDown);

                out_flag = 1;
                exit(1);
                ExitThread(1);
            }
            else if (wcscmp(command, _T("tracks")) == 0) {
                if (value <= 8 && value > 0) {
                    if (ChangeNumTracks(value) == 1)
                        _tprintf(_T("[GLOBAL] Número de estradas alterado!"));
                }
                else {
                    _tprintf(_T("[SERVER] Valor Inválido\n"));
                }
            }
            else if (wcscmp(command, _T("vspeed")) == 0) {
                if (ChangeSpeed(value) == 1)
                    _tprintf(_T("[GLOBAL] Velocidade inícial das viaturas alterado!"));
            }
            else if (wcscmp(command, _T("list")) == 0) {
                _tprintf(_T("[SERVER] Número de estradas Inicial: %d\n"), p->gameData->num_tracks);
                _tprintf(_T("[SERVER] Velocidade Inicial : %d\n"), p->gameData->vehicle_speed);
            }
            else if (wcscmp(command, _T("pause")) == 0) {

                if (SuspendThread(p->thIDs[1]) == (DWORD)-1) {
                    _tprintf(L"[SERVER] Erro ao suspender a thread. Código de erro: %d\n", GetLastError());
                    return 1;
                }

                for (int i = 0; i < p->gameData->num_tracks; i++) {
                    if (SuspendThread(p->move_threads[i]) == (DWORD)-1) {
                        _tprintf(L"[SERVER] Erro ao suspender a thread. Código de erro: %d\n", GetLastError());
                        return 1;
                    }

                }

                _tprintf(L"Jogo Pausado!\n");

            }
            else if (wcscmp(command, _T("resume")) == 0) {

                if (ResumeThread(p->thIDs[1]) == (DWORD)-1) {
                    _tprintf(L"Erro ao retomar a execução da thread. Código de erro: %d\n", GetLastError());
                    return 1;
                }

                for (int i = 0; i < p->gameData->num_tracks; i++) {
                    if (ResumeThread(p->move_threads[i]) == (DWORD)-1) {
                        _tprintf(L"[SERVER] Erro ao suspender a thread. Código de erro: %d\n", GetLastError());
                        return 1;
                    }

                }

                _tprintf(L"Jogo Iniciado!\n");

            }
            else if (wcscmp(command, _T("restart")) == 0) {

                FillGameDefaults(p->gameData);
                _tprintf(L"Jogo reiniciado\n");

            }
            else {
                _tprintf(L"[SERVER] Comando Inválido!\n");
            }

        }

    }

    ExitThread(1);
}


int _tmain(int argc, TCHAR* argv[]) {

    UNICODE_INITIALIZER();

    HANDLE shutDownEvent = CreateEvent(NULL, TRUE, FALSE, SERVER_SHUTDOWN);

    HANDLE verifySemaphore = CreateSemaphore(NULL, SERVER_LIMIT_USERS, SERVER_LIMIT_USERS, SERVER_SEMAPHORE);

    if (verifySemaphore == NULL)
        return 1;


    //verifing if any server was running

    if (WaitForSingleObject(verifySemaphore, 0L) != WAIT_OBJECT_0) {
        _tprintf(SERVER_RUNNING_MSG); Sleep(TIMEOUT);
        return -1;
    }

  
    //Getting dll to initialize our shared memory ...

    HINSTANCE hinstDLL = LoadLibrary(TEXT("sharedMemoryInterator.dll"));
    
    if (hinstDLL != NULL) {
        INIT_GAME_MEMORY init = (INIT_GAME_MEMORY)GetProcAddress(hinstDLL, "initialize_game_shared_memory");
        if (init != NULL) {
            init();
        }

        INIT_CMDS_MEMORY initCmd = (INIT_CMDS_MEMORY)GetProcAddress(hinstDLL, "initialize_cmds_shared_memory");
        if (initCmd != NULL) {
            initCmd();
        }

        FreeLibrary(hinstDLL);
    }

    thParams structTh = { 0 };
    moveParam structMove[8] = { 0 };
    DWORD dwIDThreads[MAX_THREADS];
    HANDLE hThreads[MAX_THREADS];
    HANDLE hMovementCars[8];

    //Filling game defaults

    game gameData = FillRegistryValues();
    FillGameDefaults(&gameData);

    //filling threads structures

    structTh.gameData = &gameData;
    structTh.thIDs = &hThreads;
    structTh.move_threads = &hMovementCars;
    structTh.hBlock = CreateMutex(NULL, FALSE, MUTEX_COMMAND_ACCESS);
    structTh.hRead = CreateSemaphore(NULL, 0, BUFFER_SIZE, READ_SEMAPHORE);
    structTh.hWrite = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, WRITE_SEMAPHORE);

    if (structTh.hBlock == NULL || structTh.hWrite == NULL || structTh.hRead == NULL) {
        _tprintf(L"[ERROR] creating handles!\n");

        return -1;
    }

    //Creating named pipe with overlapped

    for (int i = 0; i < N_CLIENTS; i++) {

        // aqui passamos a constante FILE_FLAG_OVERLAPPED para o named pipe aceitar comunicações assincronas
        HANDLE hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            N_CLIENTS,
            sizeof(api_pipe),
            sizeof(api_pipe),
            1000,
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            _tprintf(TEXT("[ERROR](CreateNamedPipe)"));
            exit(-1);
        }

        HANDLE hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (hEventTemp == NULL) {
            _tprintf(TEXT("[ERROR] create event (MAIN)\n"));
            return -1;
        }

        structTh.pipe[i].hPipe = hPipe;
        structTh.pipe[i].active = FALSE;

        ZeroMemory(&structTh.pipe[i].overlap, sizeof(structTh.pipe[i].overlap));

        structTh.pipe[i].overlap.hEvent = hEventTemp;
        structTh.hEvents[i] = hEventTemp;

        ConnectNamedPipe(hPipe, &structTh.pipe[i].overlap);
    }

    InitializeCriticalSection(&structTh.critical);

    //creating threads 

    hThreads[0] = CreateThread(NULL, 0, input_thread, &structTh, 0, &dwIDThreads[0]);
    hThreads[1] = CreateThread(NULL, 0, game_manager, &structTh, 0, &dwIDThreads[1]);
    hThreads[2] = CreateThread(NULL, 0, cmd_receiver, &structTh, 0, &dwIDThreads[2]);
    hThreads[3] = CreateThread(NULL, 0, connect_clients, &structTh, 0, &dwIDThreads[3]);

    for (int i = 0; i < gameData.num_tracks; i++) {

        structMove->critical = structTh.critical;
        structMove[i].gameData = &gameData;
        structMove[i].track = i;
        hMovementCars[i] = CreateThread(NULL, 0, move_cars, &structMove[i], 0, NULL);

    }


    //waiting threads to finish
    WaitForMultipleObjects(MAX_THREADS, &hThreads, TRUE, INFINITE);
    WaitForMultipleObjects(gameData.num_tracks, &hMovementCars, TRUE, INFINITE);

    //closing handles

    DeleteCriticalSection(&structTh.critical);
    CloseHandle(verifySemaphore);
    SetEvent(shutDownEvent);
    CloseHandle(shutDownEvent);
    CloseHandle(structTh.hWrite);
    CloseHandle(structTh.hBlock);
    CloseHandle(structTh.hRead);

    for (int i = 0; i < N_CLIENTS; i++) {

        if (!DisconnectNamedPipe(structTh.pipe[i].hPipe)) {
            _tprintf(TEXT("[ERRO](DisconnectNamedPipe)"));
            exit(-1);
        }
    }

    return 0;
}

void invertOrientation(game* g) {

    for (int i = 0; i < g->num_tracks; i++) {
        for (int j = 0; j < g->n_cars_per_track; j++) {
            if (g->cars[i][j].orientation == 1) {
                g->cars[i][j].orientation = 2;
            }
            else {
                g->cars[i][j].orientation = 1;
            }

        }
    }

}

int setObstacle(game* g) {

    srand(time(NULL));
    int x;
    int y;


    do {
        x = rand() % W_GAME;
        y = rand() % g->num_tracks;

        for (int i = 0; i < H_GAME; i++) {
            for (int j = 0; j < W_GAME; j++)
                if (g->table[y][x] == 'O')
                    x = 0;

        }

    } while (x == 0 || y == 0);

    g->table[y][x] = 'O';


    return 1;

}

void setFrog(game * g, int id) {

    srand(time(NULL));

    int k = 0;

    do {
        g->frogs[id].x = 0;
        g->frogs[id].y = rand() % W_GAME;

        if (g->table[g->frogs[id].x][g->frogs[id].y] != 'S') {

            k = 1;
        }
  

    } while (k == 0);

    g->table[g->frogs[id].x][g->frogs[id].y] = 'S';


}

int FillGameDefaults(game* g) {

    srand(time(NULL));
    int direction = 0;

    g->n_cars_per_track = rand() % MAX_VEHICLES;
    if (g->n_cars_per_track == 0)
        g->n_cars_per_track = DEFAULT;

    memset(g->table, ' ', sizeof(g->table));


    //Colocar os carros
    for (int i = 0; i < g->num_tracks; i++) {

        g->track_speed[i] = rand() % g->vehicle_speed;
        if (g->track_speed[i] == 0)
            g->track_speed[i] = 1;

        direction = rand() % 2;

        for (int j = 0; j < g->n_cars_per_track; j++) {
            g->cars[i][j].orientation = direction;
            g->cars[i][j].x = i + 1;
            g->cars[i][j].y = rand() % W_GAME;

            if (direction == 1)
                g->table[g->cars[i][j].x][g->cars[i][j].y] = '>';
            else
                g->table[g->cars[i][j].x][g->cars[i][j].y] = '<';
        }
    }

    return 1;
}

int ChangeNumTracks(INT value) {

    HKEY key;
    DWORD dwValue;
    int ret = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_WRITE, &key) == ERROR_SUCCESS) {

        DWORD newValue = (DWORD)value;

        LONG err = RegSetValueEx(key, N_TRACKS_ATT, 0, REG_DWORD, &newValue, sizeof(newValue));

        if (err != ERROR_SUCCESS) {
            _tprintf(_T("[SERVER] ERRO ao atualizar os valores do registry !"));
            ret = 1;
        }


    }

    RegCloseKey(key);
    return ret;

}

int ChangeSpeed(INT value) {

    HKEY key;
    DWORD dwValue;
    int ret = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, KEY_WRITE, &key) == ERROR_SUCCESS) {

        DWORD newValue = (DWORD)value;

        LONG err = RegSetValueEx(key, START_SPEED_ATT, 0, REG_DWORD, &newValue, sizeof(newValue));

        if (err != ERROR_SUCCESS) {
            _tprintf(_T("[SERVER] ERRO ao atualizar os valores do registry !"));
            ret = 1;
        }


    }

    RegCloseKey(key);
    return ret;

}

game FillRegistryValues() {

    game gameData;
    HKEY key, KeySuccess;
    DWORD dwValue;
    WCHAR wcValue[TAM];
    PVOID pvData = wcValue;

    //Trata dos dados das estradas para o registry

    if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_READ, &key) == ERROR_SUCCESS) {

        DWORD size = sizeof(wcValue);
        LONG err = RegGetValue(key, NULL, N_TRACKS_ATT, RRF_RT_ANY, NULL, pvData, &size);

        if (err == ERROR_SUCCESS) {
            DWORD convert = *(DWORD*)pvData;
            gameData.num_tracks = (INT)convert;
        }
        else {
            _tprintf(_T("[SERVER] ERRO ao receber os valores do registry !"));
        }

    }
    else {

        do {
            _tprintf(COUT_TRACKS);
            _tscanf_s(_T("%u"), &dwValue, sizeof(dwValue));
            gameData.num_tracks = (INT)dwValue;

        } while (gameData.num_tracks > (INT)GAME_MAX_TRACKS || gameData.num_tracks < (INT)1);

        if (RegCreateKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(HKEY_CURRENT_USER, N_TRACKS, 0, KEY_ALL_ACCESS, &KeySuccess) == ERROR_SUCCESS)
                RegSetValueEx(KeySuccess, N_TRACKS_ATT, 0, REG_DWORD, &dwValue, sizeof(dwValue));

            RegCloseKey(KeySuccess);
        }

    }

    RegCloseKey(key);

    //Trata dos dados da velocidade inicial para o registry

    if (RegOpenKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, KEY_READ, &key) == ERROR_SUCCESS) {

        DWORD size = sizeof(wcValue);
        LONG err = RegGetValue(key, NULL, START_SPEED_ATT, RRF_RT_ANY, NULL, pvData, &size);

        if (err == ERROR_SUCCESS) {
            DWORD convert = *(DWORD*)pvData;
            gameData.vehicle_speed = (INT)convert;
        }
        else {
            _tprintf(_T("[SERVER] ERRO ao receber os valores do registry !"));
        }

    }
    else {

        _tprintf(COUT_SPEED);
        _tscanf_s(_T("%u"), &dwValue, sizeof(dwValue));
        gameData.vehicle_speed = (INT)dwValue;

        if (RegCreateKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(HKEY_CURRENT_USER, START_SPEED, 0, KEY_ALL_ACCESS, &KeySuccess) == ERROR_SUCCESS)
                RegSetValueEx(KeySuccess, START_SPEED_ATT, 0, REG_DWORD, &dwValue, sizeof(dwValue));

            RegCloseKey(KeySuccess);
        }

    }

    RegCloseKey(key);

    return gameData;
}

void UNICODE_INITIALIZER() {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    _tprintf(TEXT("-----------SERVER--------------\n"));
}