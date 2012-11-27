#include <windows.h>
#include <math.h>

#define NUM    1000
#define TWOPI  (2 * 3.14159)

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

POINT		apt[NUM];


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("Display Wave");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;
     
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppName;
          
	if (!RegisterClass (&wndclass))
	{
		MessageBox(NULL, TEXT ("Program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}
     
	hwnd = CreateWindow (szAppName, TEXT ("Display Wave"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
     
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int	cxClient, cyClient;
	static HPEN	green_pen;
	static HGDIOBJ	old_pen;
	HDC		hdc;
	int		i;
	PAINTSTRUCT	ps;

     
	switch (message)
	{
	case WM_CREATE:
		green_pen = CreatePen(PS_SOLID, 1, RGB(0,255,0));
		return 0;
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		for (i = 0; i < NUM; i++)
		{
			apt[i].x = i * cxClient / NUM;
			apt[i].y = (int) (cyClient / 2 * (1 - sin (TWOPI * i / NUM)));
		}
		return 0;
	     
	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps);
		old_pen = SelectObject(hdc, green_pen);

		// Center line
		MoveToEx (hdc, 0,        cyClient / 2, NULL);
		LineTo   (hdc, cxClient, cyClient / 2);

		Polyline(hdc, apt, NUM);
		SelectObject(hdc, old_pen);
		EndPaint(hwnd, &ps);
		return 0;
	     
	case WM_DESTROY:
		
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

