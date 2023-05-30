#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include "frog.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);


TCHAR szProgName[] = TEXT("Base");
thParams args;
OVERLAPPED overlapped;

void ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}

void WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}

DWORD WINAPI receive_thread(thParams p) {

	DWORD bytesAvailable = 0;
	api receive;

	while (1) {


		BOOL pipeHasData = PeekNamedPipe(p.pipe, NULL, 0, NULL, &bytesAvailable, NULL);

		if (pipeHasData && bytesAvailable > 0) {

			if (ReadFileEx(p.pipe, &receive, sizeof(receive), &overlapped, &ReadCompletionRoutine))
			{
				EnterCriticalSection(&p.critical);

				args.gameView.num_tracks = receive.num_tracks;
				args.myPoints = receive.points;

				for (int i = H_GAME - 1; i >= -1; i--) {
					for (int j = W_GAME - 1; j >= 0; j--) {
						args.gameView.table[i][j] = receive.table[i][j];
					}
				}

				InvalidateRect(p.mainWindow, NULL, TRUE);

				LeaveCriticalSection(&p.critical);
			}
		}
	}

	ExitThread(1);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;
	api initialize_game;
	HANDLE hThreads[1];
	InitializeCriticalSection(&args.critical);

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;

	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;

	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	wcApp.hIconSm = LoadIcon(NULL, IDI_SHIELD);

	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);

	wcApp.lpszMenuName = NULL;

	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = CreateSolidBrush(RGB(27, 40, 138));

	if (!RegisterClassEx(&wcApp))
		return(0);


	hWnd = CreateWindow(
		szProgName,
		TEXT("Frooger-Game"),
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posição x pixels (default=à direita da última)
		CW_USEDEFAULT,		// Posição y pixels (default=abaixo da última)
		1280,		// Largura da janela (em pixels)
		720,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira, 
		// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da instância do programa actual ("hInst" é 
		// passado num dos parâmetros de WinMain()
		0);				// Não há parâmetros adicionais para a janela

	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);

	if (!WaitNamedPipe(PIPE_NAME, 10000)) {
		if (MessageBox(hWnd, L"Server not Running", L"Server not running", MB_OK | MB_ICONERROR) == IDOK) {
			exit(-1);
		}
	}

	HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	args.mainWindow = hWnd;
	args.pipe = hPipe;
	args.status = 4;


	int result = MessageBox(hWnd, L"Do you want to play Multyplayer?", L"Game Mode Selection", MB_YESNO | MB_ICONQUESTION);

	if (result == IDYES) {
		initialize_game.msg = 2;
	}
	else if (result = IDNO) {
		initialize_game.msg = 1;
	}

	WriteFile(hPipe, &initialize_game, sizeof(initialize_game), 0, NULL);

	ZeroMemory(&overlapped, sizeof(overlapped));

	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	hThreads[0] = CreateThread(NULL, 0, receive_thread, &args, 0, 0);
	args.receiver = hThreads[0];

	while (GetMessage(&lpMsg, NULL, 0, 0)) {

		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);

	}

	return((int)lpMsg.wParam);
}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {


	HDC hdc = NULL; RECT rect;
	PAINTSTRUCT ps;
	api send;

	switch (messg) {

	case WM_CLOSE:

		if (MessageBox(hWnd, L"DO YOU WANT TO QUIT?", L"Confirmation", MB_YESNO | MB_ICONQUESTION) == IDYES) {
			send.key = 1;
			WriteFileEx(args.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
			DestroyWindow(hWnd);
			ExitProcess(-1);
		}
		break;

	case WM_DESTROY:

		PostQuitMessage(0);
		break;


	case WM_KEYDOWN:

		switch (wParam)
		{
		case VK_LEFT:
			send.key = 3;
			WriteFileEx(args.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
			break;
		case VK_RIGHT:
			send.key = 4;
			WriteFileEx(args.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
			break;
		case VK_UP:
			send.key = 2;
			WriteFileEx(args.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
			break;
		case VK_ESCAPE:

			SuspendThread(args.receiver);
			if (MessageBox(hWnd, L"Game paused!", L"game stopped", MB_OK | MB_ICONASTERISK) == IDOK) {
				ResumeThread(args.receiver);
			}
			break;
		}


		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &rect);

		HDC hdc = GetDC(hWnd);

		EnterCriticalSection(&args.critical);

		paint_game_zone(hdc, rect);

		LeaveCriticalSection(&args.critical);

		ReleaseDC(hWnd, hdc);

		EndPaint(hWnd, &ps);


		break;

	default:

		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}

	return(0);
}

void paint_game_zone(HDC hdc, RECT rect) {

	static int counter= 0;

	int width = 50;
	int x = 100;
	int height = 50;
	int y = 80;

	// Desenhe o conteúdo no buffer off-screen
	for (int i = H_GAME - 1; i >= 0; i--) {
		for (int j = W_GAME - 1; j >= 0; j--) {
			wchar_t c = args.gameView.table[i][j];

			if (i == args.gameView.num_tracks + 1) {
				c = L'_';
			}
			
			if (i == 0 && args.gameView.table[i][j] != L'S') {
				c = L'_';
			}

			if ((j == 0 || j == W_GAME - 1) && i < args.gameView.num_tracks + 2) {
				c = L'|';
			}

			
			RECT cellRect;
			cellRect.left = x + (W_GAME - 1 - j) * width;
			cellRect.top = y + (H_GAME - 1 - i) * height;
			cellRect.right = cellRect.left + width;
			cellRect.bottom = cellRect.top + height;

			if (c == L'S') {
				if (i == 0) {
					FillRect(hdc, &cellRect, CreateSolidBrush(RGB(0, 0, 0)));
					SetTextColor(hdc, RGB(255, 255, 0));
					SetBkColor(hdc, RGB(0, 0, 0));
				}
				else {
					SetTextColor(hdc, RGB(255, 255, 0));
					SetBkColor(hdc, RGB(27, 40, 138));
				}
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
			}
			else if(c == L'|' || c == L'_') {

				FillRect(hdc, &cellRect, CreateSolidBrush(RGB(0, 0, 0)));
				SetTextColor(hdc, RGB(0, 0, 0));
				SetBkColor(hdc, RGB(0, 0, 0));
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
	
			}
			else if (c == L'<' || c == L'>') {
				SetTextColor(hdc, RGB(255, 0, 0));
				SetBkColor(hdc, RGB(27, 40, 138));
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
		
			}
			else if (c == L'O') {
				FillRect(hdc, &cellRect, CreateSolidBrush(RGB(0, 0, 0)));
				SetTextColor(hdc, RGB(0, 0, 0));
				SetBkColor(hdc, RGB(0, 0, 0));
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
			}


		}
	}

}