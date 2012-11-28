#include <windows.h>
#include <math.h>
#include <process.h>

#define NUM    4096
#define TWOPI  (2 * 3.14159)

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

POINT		apt[NUM];
int		cxClient = 1;
int		cyClient = 1;

#define LENGTH 4096
char data1[LENGTH];
char data2[LENGTH];
char data3[LENGTH];

void play_wave(WAVEFORMATEX *format, char *data, int length)
{
	HWAVEOUT	hWaveOut;
	WAVEHDR		wavehdr = {0};

	wavehdr.lpData = data;
	wavehdr.dwBufferLength = length;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, format, 0, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));
	Sleep( 1000 * length / (format->nSamplesPerSec * format->nChannels * (format->wBitsPerSample / 8)) );
}


int get_wave(WAVEFORMATEX *format, char *data, int length)
{
	HWAVEIN		hwavein;
	WAVEHDR		wavehdr = {0};

	format->wFormatTag = WAVE_FORMAT_PCM;
	format->nChannels = 1;
	format->nSamplesPerSec = 44100;
	format->nAvgBytesPerSec = 44100 * 2;
	format->nBlockAlign = 2;
	format->wBitsPerSample = 16;
	format->cbSize = 0;

	wavehdr.lpData = data;
	wavehdr.dwBufferLength = length;


	if ( waveInOpen(&hwavein, WAVE_MAPPER, format, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		printf("waveInOpen failed\n");
		return -1;
	}

	if ( waveInPrepareHeader(hwavein, &wavehdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveInPrepareHeader failed\n");
		return -1;
	}

	if ( waveInAddBuffer(hwavein, &wavehdr, length) != MMSYSERR_NOERROR)
	{
		printf("waveInAddBuffer failed\n");
		return -1;
	}

	waveInStart(hwavein);
	while ( (wavehdr.dwFlags & WHDR_DONE) == 0)
		Sleep(1000);

	waveInReset(hwavein);
	waveInClose(hwavein);
	return 0;
}

int distortion(short int *data, int length, float gain, int clip)
{
	int i;
	int value;

	for(i = 0; i < length; i++)
	{
		value = data[i] * gain;
		if (value > clip)
			data[i] = clip;
		else if(value < -clip)
			data[i] = -clip;
		else
			data[i] = value;
	}

	return 0;
}

void ProcessBuffer(int index, short int *data, int length)
{
	int i;

	for (i = 0; i < NUM; i++)
	{
		apt[i].x = i * cxClient / NUM;
		if (index == 1)
			apt[i].y = (int) (cyClient / 2 * (1 - data1[i]));
		else if (index == 2)
			apt[i].y = (int) (cyClient / 2 * (1 - data2[i]));
		else
			apt[i].y = (int) (cyClient / 2 * (1 - data3[i]));
	}


//	distortion(data, length, 100.0, 24575);
}

int stream_wave()
{
	HWAVEOUT	hWaveOut;
	HWAVEIN		hWaveIn;

	WAVEHDR		wavehdr1_in = {0};
	WAVEHDR		wavehdr1_out = {0};

	WAVEHDR		wavehdr2_in = {0};
	WAVEHDR		wavehdr2_out = {0};

	WAVEHDR		wavehdr3_in = {0};
	WAVEHDR		wavehdr3_out = {0};

	WAVEFORMATEX	wformat;

	wformat.wFormatTag = WAVE_FORMAT_PCM;
	wformat.nChannels = 1;
	wformat.nSamplesPerSec = 44100;
	wformat.nAvgBytesPerSec = 44100 * 2;
	wformat.nBlockAlign = 2;
	wformat.wBitsPerSample = 16;
	wformat.cbSize = 0;

	wavehdr1_in.lpData = data1;
	wavehdr1_out.lpData = data1;
	wavehdr1_in.dwBufferLength = LENGTH;
	wavehdr1_out.dwBufferLength = LENGTH;

	wavehdr2_in.lpData = data2;
	wavehdr2_out.lpData = data2;
	wavehdr2_in.dwBufferLength = LENGTH;
	wavehdr2_out.dwBufferLength = LENGTH;


	wavehdr3_in.lpData = data3;
	wavehdr3_out.lpData = data3;
	wavehdr3_in.dwBufferLength = LENGTH;
	wavehdr3_out.dwBufferLength = LENGTH;

	if ( waveInOpen(&hWaveIn, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		printf("waveInOpen failed\n");
		return -1;
	}

	if ( waveInPrepareHeader(hWaveIn, &wavehdr1_in, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveInPrepareHeader failed\n");
		return -1;
	}
	if ( waveInPrepareHeader(hWaveIn, &wavehdr2_in, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveInPrepareHeader failed\n");
		return -1;
	}
	if ( waveInPrepareHeader(hWaveIn, &wavehdr3_in, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveInPrepareHeader failed\n");
		return -1;
	}

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL);

	if ( waveOutPrepareHeader(hWaveOut, &wavehdr1_out, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveOutPrepareHeader failed\n");
		return -1;
	}

	if ( waveOutPrepareHeader(hWaveOut, &wavehdr2_out, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveOutPrepareHeader failed\n");
		return -1;
	}

	if ( waveOutPrepareHeader(hWaveOut, &wavehdr3_out, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveOutPrepareHeader failed\n");
		return -1;
	}

	if ( waveInAddBuffer(hWaveIn, &wavehdr1_in, LENGTH) != MMSYSERR_NOERROR)
	{
		printf("waveInAddBuffer failed\n");
		return -1;
	}
	waveInStart(hWaveIn);
	while (1)
	{


		if ( waveInAddBuffer(hWaveIn, &wavehdr2_in, LENGTH) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}

//		Sleep( 1000 * LENGTH / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );
		while ( (wavehdr1_in.dwFlags & WHDR_DONE) == 0)
			Sleep(0);

		ProcessBuffer(1, (short int *)data1, LENGTH / 2);
		waveOutWrite(hWaveOut, &wavehdr1_out, sizeof(WAVEHDR));

//		Sleep( 1000 * LENGTH / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );

		if ( waveInAddBuffer(hWaveIn, &wavehdr3_in, LENGTH) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}

		while ( (wavehdr2_in.dwFlags & WHDR_DONE) == 0)
			Sleep(0);


		ProcessBuffer(2, (short int *)data2, LENGTH / 2);
		waveOutWrite(hWaveOut, &wavehdr2_out, sizeof(WAVEHDR));

//		Sleep( 1000 * LENGTH / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );
		if ( waveInAddBuffer(hWaveIn, &wavehdr1_in, LENGTH) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}

		while ( (wavehdr3_in.dwFlags & WHDR_DONE) == 0)
			Sleep(0);

		ProcessBuffer(3, (short int *)data3, LENGTH / 2);
		waveOutWrite(hWaveOut, &wavehdr3_out, sizeof(WAVEHDR));

	}


	waveInReset(hWaveIn);
	waveInClose(hWaveIn);
	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);
	return 0;
}

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




unsigned int __stdcall stream_thread(void *arg)
{
	stream_wave();
	return 0;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	static HPEN	green_pen;
	static HGDIOBJ	old_pen;
	static HANDLE	hThread;
	HDC		hdc;
	int		i;
	PAINTSTRUCT	ps;

     
	switch (message)
	{
	case WM_CREATE:
		green_pen = CreatePen(PS_SOLID, 1, RGB(0,255,0));
		hThread = (HANDLE)_beginthreadex(0, 0, stream_thread, 0, 0, 0);
		return 0;
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
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

