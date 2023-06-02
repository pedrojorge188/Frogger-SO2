#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include "frog.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("FroggerGame");
thParams args;
OVERLAPPED overlapped;

void ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}

void WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {}

DWORD WINAPI receive_server_infos(thParams p) {

	DWORD bytesAvailable = 0;
	api receive;

	while (1) {


		if (WaitForSingleObject(OpenEventW(EVENT_ALL_ACCESS, FALSE, SERVER_SHUTDOWN), INFINITE) == WAIT_OBJECT_0) {

			if (MessageBox(args.mainWindow, L"Server shutdown", L"Server not running", MB_OK | MB_ICONERROR) == IDOK) {
				exit(-1);
			}

		}
	}

	ExitThread(1);
}

DWORD WINAPI receive_thread(thParams p) {

	DWORD bytesAvailable = 0;
	api receive = { 0 };
	api send = { 0 };
	send.key = -1;

	WriteFileEx(p.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);

	while (1) {

		BOOL pipeHasData = PeekNamedPipe(p.pipe, NULL, 0, NULL, &bytesAvailable, NULL);

		if (pipeHasData && bytesAvailable > 0) {

			if (ReadFileEx(p.pipe, &receive, sizeof(receive), &overlapped, &ReadCompletionRoutine))
			{
				EnterCriticalSection(&p.critical);

				p.gameView.num_tracks = receive.num_tracks;
				p.myPoints = receive.points;

				for (int i = H_GAME - 1; i >= -1; i--) {
					for (int j = W_GAME - 1; j >= 0; j--) {
						p.gameView.table[i][j] = receive.table[i][j];
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
	HANDLE hThreads[2];
	wchar_t szExePath[150];
	GetModuleFileName(NULL, szExePath, 150);

	wchar_t* pLastSlash = wcsrchr(szExePath, L'\\');
	if (pLastSlash)
		*(pLastSlash + 1) = L'\0';

	wcscat_s(szExePath, sizeof(szExePath), L"frog.ico");

	InitializeCriticalSection(&args.critical);

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;

	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;

	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	wcApp.hIcon = (HICON)LoadImage(NULL, szExePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	wcApp.hIconSm = (HICON)LoadImage(NULL, szExePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);

	wcApp.lpszMenuName = NULL;

	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

	if (!RegisterClassEx(&wcApp))
		return(0);


	hWnd = CreateWindow(
		szProgName,
		TEXT("Frooger-Game"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,    // Largura da janela (tela inteira)
		800,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL,
		(HINSTANCE)hInst,
		0);


	HMENU hSubMenu = CreatePopupMenu();
	HMENU hMenubar = CreateMenu();

	HMENU hMenu = CreateMenu();
	HMENU hMenuStatus = CreateMenu();

	AppendMenu(hMenu, MF_STRING, 1, L"Bitmap 1");
	AppendMenu(hMenu, MF_STRING, 2, L"Bitmap 2");

	AppendMenu(hMenuStatus, MF_STRING, 3, L"Points");
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"Bitmaps");
	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenuStatus, L"Status");

	SetMenu(hWnd, hMenubar);

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
	args.status = 0;

	ReadFile(hPipe, &initialize_game, sizeof(initialize_game), 0, NULL);

	if (initialize_game.msg == 0) {

			int result = MessageBox(hWnd, L"Do you want to play Multyplayer?", L"Game Mode Selection", MB_YESNO | MB_ICONQUESTION);

		if (result == IDYES) {
			initialize_game.msg = 2;
		}
		else if (result = IDNO) {
			initialize_game.msg = 1;
		}

		WriteFile(hPipe, &initialize_game, sizeof(initialize_game), 0, NULL);

	}

	ZeroMemory(&overlapped, sizeof(overlapped));

	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	hThreads[0] = CreateThread(NULL, 0, receive_thread, &args, 0, 0);
	hThreads[1] = CreateThread(NULL, 0, receive_server_infos, &args, 0, 0);

	args.receiver = hThreads[0];

	while (GetMessage(&lpMsg, NULL, 0, 0)) {

		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);

	}

	return((int)lpMsg.wParam);

	WaitForMultipleObjects(2, &hThreads, TRUE, INFINITE);
}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	HDC hdc = NULL; RECT rect;
	PAINTSTRUCT ps;
	api send;

	switch (messg) {
	case WM_COMMAND:
	{

		if (wParam == 3)
		{
			wchar_t msg[100];
			swprintf(msg, 100, L"My Points: %d", args.myPoints);

			MessageBox(hWnd, msg, L"Points", MB_OK);
		}
		break;
	}
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

	case WM_LBUTTONDOWN:

		send.key = -1;
		WriteFileEx(args.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
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

		int width = 800 / W_GAME;
		int x = 800 - width;
		int height = 800 / H_GAME;
		int y = 800 - height;

		for (int i = H_GAME - 1; i >= 0; i--) {
			for (int j = W_GAME - 1; j >= 0; j--) {

				RECT cellRect;
				cellRect.left = x - j * width;
				cellRect.top = y - i * height;
				cellRect.right = cellRect.left + width;
				cellRect.bottom = cellRect.top + height;

				if (i == 0 || i >= args.gameView.num_tracks + 1) {
					HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
					FillRect(hdc, &cellRect, blueBrush);
					DeleteObject(blueBrush);
				}

			}
		}


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

	int width = 800 / W_GAME;
	int x = 800 - width;
	int height = 800 / H_GAME;
	int y = 800 - height;

	for (int i = H_GAME - 1; i >= 0; i--) {
		for (int j = W_GAME - 1; j >= 0; j--) {

			wchar_t c = args.gameView.table[i][j];

			RECT cellRect;
			cellRect.left = x - j * width;
			cellRect.top = y - i * height;
			cellRect.right = cellRect.left + width;
			cellRect.bottom = cellRect.top + height;

			if (c == L'S') {
				if (i == 0)
					SetBkColor(hdc, RGB(0, 0, 255));
				else
					SetBkColor(hdc, RGB(0, 0, 0));
				SetTextColor(hdc, RGB(255, 0, 0));
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
			}
			else if (c == L'<' || c == L'>') {
				SetTextColor(hdc, RGB(255, 255, 255));
				SetBkColor(hdc, RGB(0, 0, 0));
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);

			}
			else if (c == L'O') {
				FillRect(hdc, &cellRect, CreateSolidBrush(RGB(255, 255, 255)));
				SetTextColor(hdc, RGB(255, 255, 255));
				SetBkColor(hdc, RGB(255, 255, 255));
				DrawTextW(hdc, &c, 1, &cellRect, DT_SINGLELINE | DT_CENTER | DT_NOCLIP);
			}

		}
	}

}