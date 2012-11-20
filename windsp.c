#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

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

void ProcessBuffer(short int *data, int length)
{
//	distortion(data, length, 100.0, 24575);
}



#define LENGTH 4096
int stream_wave()
{
	char data1[LENGTH];
	char data2[LENGTH];
	char data3[LENGTH];

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

		ProcessBuffer((short int *)data1, LENGTH / 2);
		waveOutWrite(hWaveOut, &wavehdr1_out, sizeof(WAVEHDR));

//		Sleep( 1000 * LENGTH / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );

		if ( waveInAddBuffer(hWaveIn, &wavehdr3_in, LENGTH) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}

		while ( (wavehdr2_in.dwFlags & WHDR_DONE) == 0)
			Sleep(0);


		ProcessBuffer((short int *)data2, LENGTH / 2);
		waveOutWrite(hWaveOut, &wavehdr2_out, sizeof(WAVEHDR));

//		Sleep( 1000 * LENGTH / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );
		if ( waveInAddBuffer(hWaveIn, &wavehdr1_in, LENGTH) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}

		while ( (wavehdr3_in.dwFlags & WHDR_DONE) == 0)
			Sleep(0);

		ProcessBuffer((short int *)data3, LENGTH / 2);
		waveOutWrite(hWaveOut, &wavehdr3_out, sizeof(WAVEHDR));

	}


	waveInReset(hWaveIn);
	waveInClose(hWaveIn);
	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);
	return 0;
}







int main(void)
{
	stream_wave();
	return 0;
}


