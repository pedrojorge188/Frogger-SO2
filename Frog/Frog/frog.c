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
	BITMAP bmp[7];
	HBITMAP hBmp[7];
	HDC bmpDC[7];

	DWORD bytesAvailable = 0;
	api receive = { 0 };
	api send = { 0 };
	send.key = -1;

	WriteFileEx(p.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);

	HDC hdc = GetDC(p.mainWindow);
	RECT rect;

	GetClientRect(p.mainWindow, &rect);

	HDC hdcBuffer = CreateCompatibleDC(hdc);

	hBmp[0] = (HBITMAP)LoadImage(NULL, TEXT("gameFrog.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[0], sizeof(bmp[0]), &bmp[0]);
	bmpDC[0] = CreateCompatibleDC(hdc);
	GetObject(hBmp[0], sizeof(BITMAP), &bmp[0]);

	hBmp[1] = (HBITMAP)LoadImage(NULL, TEXT("car1.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[1], sizeof(bmp[1]), &bmp[1]);
	bmpDC[1] = CreateCompatibleDC(hdc);
	GetObject(hBmp[1], sizeof(BITMAP), &bmp[1]);

	hBmp[2] = (HBITMAP)LoadImage(NULL, TEXT("car2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[2], sizeof(bmp[2]), &bmp[2]);
	bmpDC[2] = CreateCompatibleDC(hdc);
	GetObject(hBmp[2], sizeof(BITMAP), &bmp[2]);

	hBmp[3] = (HBITMAP)LoadImage(NULL, TEXT("gameFrog2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[3], sizeof(bmp[3]), &bmp[3]);
	bmpDC[3] = CreateCompatibleDC(hdc);
	GetObject(hBmp[3], sizeof(BITMAP), &bmp[3]);

	hBmp[4] = (HBITMAP)LoadImage(NULL, TEXT("car4.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[4], sizeof(bmp[4]), &bmp[4]);
	bmpDC[4] = CreateCompatibleDC(hdc);
	GetObject(hBmp[4], sizeof(BITMAP), &bmp[4]);

	hBmp[5] = (HBITMAP)LoadImage(NULL, TEXT("car3.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[5], sizeof(bmp[5]), &bmp[5]);
	bmpDC[5] = CreateCompatibleDC(hdc);

	GetObject(hBmp[4], sizeof(BITMAP), &bmp[4]);
	hBmp[6] = (HBITMAP)LoadImage(NULL, TEXT("wall.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hBmp[6], sizeof(bmp[6]), &bmp[6]);
	bmpDC[6] = CreateCompatibleDC(hdc);
	GetObject(hBmp[6], sizeof(BITMAP), &bmp[6]);

	HBITMAP hBitmapBuffer = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
	HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcBuffer, hBitmapBuffer);

	while (1) {
		BOOL pipeHasData = PeekNamedPipe(p.pipe, NULL, 0, NULL, &bytesAvailable, NULL);
		if (pipeHasData && bytesAvailable > 0) {
			if (ReadFileEx(p.pipe, &receive, sizeof(receive), &overlapped, &ReadCompletionRoutine)) {
				EnterCriticalSection(&p.critical);

					p.gameView.num_tracks = receive.num_tracks;
					p.myPoints = receive.points;

					for (int i = H_GAME - 1; i >= -1; i--) {
						for (int j = 0; j < W_GAME; j++) {
							p.gameView.table[i][j] = receive.table[i][j];
						}
					}

					paint_game_zone(bmpDC,hBmp,  bmp, &rect ,hdcBuffer,receive);

				BitBlt(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hdcBuffer, 0, 0, SRCCOPY);
				LeaveCriticalSection(&p.critical);
			}
		}
	}

	SelectObject(hdcBuffer, hBitmapOld);
	DeleteObject(hBitmapBuffer);
	DeleteDC(hdcBuffer);

	ReleaseDC(p.mainWindow, hdc);

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

	InitializeCriticalSection(&args.critical);

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;

	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;

	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	wcApp.hIcon = (HICON)LoadImage(NULL, L"frog.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	wcApp.hIconSm = (HICON)LoadImage(NULL, L"frog.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

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
	args.bitmap = 2;

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

	case WM_CREATE:
		
		SetWindowPos(hWnd, NULL, 0, 0, 800, 800, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);

		break;
	case WM_COMMAND:
	{
		if (wParam == 3)
		{
			wchar_t msg[100];
			swprintf(msg, 100, L"My Points: %d", args.myPoints);

			MessageBox(hWnd, msg, L"Points", MB_OK);
		}
		if (wParam == 1)
		{
			
			EnterCriticalSection(&args.critical);

			args.bitmap = 1;

			LeaveCriticalSection(&args.critical);

		}
		if (wParam == 2)
		{
			
			EnterCriticalSection(&args.critical);

			args.bitmap = 2;

			LeaveCriticalSection(&args.critical);
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

	case WM_SIZE:
		send.key = 50;
		WriteFileEx(args.pipe, &send, sizeof(send), &overlapped, &WriteCompletionRoutine);
		break;

	case WM_ERASEBKGND:
		return 1;
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

		case VK_DOWN:

			send.key = 5;
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
			int height = 800 / H_GAME;

			int x = 800 - width;
			int y = 800 - height;
			
			for (int i = 0; i < H_GAME; i++) {
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

void paint_game_zone(HDC * bmpDC, HBITMAP * hBmp, BITMAP * bmp, RECT * rect, HDC hdcBuffer, api receive) {

	int width = (rect->right - rect->left) / W_GAME;
	int height = (rect->bottom - rect->top) / H_GAME;

	int x = rect->right - rect->left - width;
	int y = rect->bottom - rect->top - height;

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));

	for (int i = 0; i < H_GAME; i++) {
		for (int j = W_GAME - 1; j >= 0; j--) {
			wchar_t c = args.gameView.table[i][j];

			RECT cellRect;
			cellRect.left = x - j * width - 1;
			cellRect.top = y - i * height - 1;
			cellRect.right = cellRect.left + width;
			cellRect.bottom = cellRect.top + height;

			for (int i = H_GAME - 1; i >= -1; i--) {
				for (int j = 0; j < W_GAME; j++) {
					args.gameView.table[i][j] = receive.table[i][j];
				}
			}

			switch (c) {
			case L'S':
				if (args.bitmap == 1) {
					SelectObject(bmpDC[0], hBmp[0]);
					StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[0], 0, 0, bmp[0].bmWidth, bmp[0].bmHeight, SRCCOPY);
				}
				else {
					SelectObject(bmpDC[3], hBmp[3]);
					StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[3], 0, 0, bmp[3].bmWidth, bmp[3].bmHeight, SRCCOPY);
				}
				break;
			case L'<':
				if (args.bitmap == 1) {
					SelectObject(bmpDC[1], hBmp[1]);
					StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[1], 0, 0, bmp[1].bmWidth, bmp[1].bmHeight, SRCCOPY);
				}
				else {
					SelectObject(bmpDC[5], hBmp[5]);
					StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[5], 0, 0, bmp[5].bmWidth, bmp[5].bmHeight, SRCCOPY);
				}
				break;
			case L'>':
				if (args.bitmap == 1) {
					SelectObject(bmpDC[2], hBmp[2]);
					StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[2], 0, 0, bmp[2].bmWidth, bmp[2].bmHeight, SRCCOPY);
				}
				else {
					SelectObject(bmpDC[4], hBmp[4]);
					StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[4], 0, 0, bmp[4].bmWidth, bmp[4].bmHeight, SRCCOPY);
				}
				break;
			case L'O':
				SelectObject(bmpDC[6], hBmp[6]);
				StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[6], 0, 0, bmp[6].bmWidth, bmp[6].bmHeight, SRCCOPY);
				break;
			default:
				hBrush = CreateSolidBrush(RGB(0, 0, 0));
				SetBkColor(hdcBuffer, RGB(0, 0, 0));
				SetTextColor(hdcBuffer, RGB(0, 0, 0));
				break;
			}

			if (hBrush != NULL) {
				FillRect(hdcBuffer, &cellRect, hBrush);
				DeleteObject(hBrush);
				hBrush = NULL;
			}
		}
	}

	for (int i = 0; i < H_GAME; i++) {
		for (int j = W_GAME - 1; j >= 0; j--) {
			RECT cellRect;
			cellRect.left = x - j * width ;
			cellRect.top = y - i * height ;
			cellRect.right = cellRect.left + width;
			cellRect.bottom = cellRect.top + height;

			if (i == 0 || i >= args.gameView.num_tracks + 1) {
				HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
				FillRect(hdcBuffer, &cellRect, blueBrush);
				DeleteObject(blueBrush);

				if (args.gameView.table[i][j] == L'S') {
					if (args.bitmap == 1) {
						StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[0], 0, 0, bmp[0].bmWidth, bmp[0].bmHeight, SRCCOPY);
					}
					else {
						StretchBlt(hdcBuffer, cellRect.left, cellRect.top, width, height, bmpDC[3], 0, 0, bmp[3].bmWidth, bmp[3].bmHeight, SRCCOPY);
					}
				}
			}
		}
	}


}