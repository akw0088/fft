#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	short	format;
	short	channels;
	int	sample_rate;
	int	avg_sample_rate;
	short	align;
	short	sample_size;
} wave_t;

char *get_file(char *filename)
{
	FILE	*file;
	char	*buffer;
	int	fSize, bRead;

	file = fopen(filename, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	fSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = (char *) malloc( fSize * sizeof(char) + 1 );
	bRead = fread(buffer, sizeof(char), fSize, file);
	if (bRead != fSize)
		return 0;
	fclose(file);
	buffer[fSize] = '\0';
	return buffer;
}

int check_format(char *data, char *format)
{
	return memcmp(&data[8], format, 4);
}

char *find_chunk(char *chunk, char *id, int *length, char *end)
{
	while (chunk < end)
	{
		*length = *((int *)(chunk + 4));

		if ( memcmp(chunk, id, 4) == 0 )
			return chunk + 8;
		else
			chunk += *length + 8;
	}
	return NULL;
}

void play_wave(wave_t *format, char *data, int length)
{
	HWAVEOUT	hWaveOut;
	WAVEHDR		wavehdr = {0};
	WAVEFORMATEX	wformat;

	wformat.wFormatTag = format->format;
	wformat.nChannels = format->channels;
	wformat.nSamplesPerSec = format->sample_rate;
	wformat.nAvgBytesPerSec = format->avg_sample_rate;
	wformat.nBlockAlign = format->align;
	wformat.wBitsPerSample = format->sample_size;
	wformat.cbSize = 0;

	wavehdr.lpData = data;
	wavehdr.dwBufferLength = length;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));
	Sleep( 1000 * length / (format->sample_rate * format->channels * (format->sample_size / 8)) );
}


int get_wave(wave_t *format, char *data, int length)
{
	HWAVEIN		hwavein;
	WAVEFORMATEX	wformat;
	WAVEHDR		wavehdr = {0};

	format->format = WAVE_FORMAT_PCM;
	format->channels = 1;
	format->sample_rate = 44100;
	format->avg_sample_rate = 44100 * 2;
	format->align = 2;
	format->sample_size = 16;

	wformat.wFormatTag = format->format;
	wformat.nChannels = format->channels;
	wformat.nSamplesPerSec = format->sample_rate;
	wformat.nAvgBytesPerSec = format->avg_sample_rate;
	wformat.nBlockAlign = format->align;
	wformat.wBitsPerSample = format->sample_size;
	wformat.cbSize = 0;



	wavehdr.lpData = data;
	wavehdr.dwBufferLength = length;


	if ( waveInOpen(&hwavein, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
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
	waveInUnprepareHeader(hwavein, &wavehdr, sizeof(WAVEHDR));
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


int stream_wave()
{
	int length = 44100;
	char data1[44100];
	char data2[44100];
	char data3[44100];

	HWAVEOUT	hWaveOut;
	WAVEHDR		wavehdr1 = {0};
	WAVEHDR		wavehdr2 = {0};
	WAVEHDR		wavehdr3 = {0};
	WAVEFORMATEX	wformat;
	HWAVEIN		hwavein;

	wformat.wFormatTag = WAVE_FORMAT_PCM;
	wformat.nChannels = 1;
	wformat.nSamplesPerSec = 44100;
	wformat.nAvgBytesPerSec = 44100 * 2;
	wformat.nBlockAlign = 2;
	wformat.wBitsPerSample = 16;
	wformat.cbSize = 0;

	wavehdr1.lpData = data1;
	wavehdr1.dwBufferLength = length;
	wavehdr2.lpData = data2;
	wavehdr2.dwBufferLength = length;
	wavehdr3.lpData = data3;
	wavehdr3.dwBufferLength = length;


	if ( waveInOpen(&hwavein, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		printf("waveInOpen failed\n");
		return -1;
	}

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL);


	while (1)
	{
		// Get buffer one
		if ( waveInPrepareHeader(hwavein, &wavehdr1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveInPrepareHeader failed\n");
			return -1;
		}
	
		if ( waveInAddBuffer(hwavein, &wavehdr1, length) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}
	
		waveInStart(hwavein);

		Sleep( 1000 * length / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );
		while ( (wavehdr1.dwFlags & WHDR_DONE) == 0)
			Sleep(10);

		waveInUnprepareHeader(hwavein, &wavehdr1, sizeof(WAVEHDR));
	
	
		waveOutUnprepareHeader(hWaveOut, &wavehdr3, sizeof(WAVEHDR));

		ProcessBuffer((short int *)data1, length / 2);

		if ( waveOutPrepareHeader(hWaveOut, &wavehdr1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveOutPrepareHeader failed\n");
			return -1;
		}

		waveOutWrite(hWaveOut, &wavehdr1, sizeof(WAVEHDR));

		// Get buffer two
		if ( waveInPrepareHeader(hwavein, &wavehdr2, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveInPrepareHeader failed\n");
			return -1;
		}
	
		if ( waveInAddBuffer(hwavein, &wavehdr2, length) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}
	
		waveInStart(hwavein);
		Sleep( 1000 * length / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );
		while ( (wavehdr2.dwFlags & WHDR_DONE) == 0)
			Sleep(10);
		waveInUnprepareHeader(hwavein, &wavehdr2, sizeof(WAVEHDR));
	
		waveOutUnprepareHeader(hWaveOut, &wavehdr1, sizeof(WAVEHDR));


		ProcessBuffer((short int *)data2, length / 2);

		if (waveOutPrepareHeader(hWaveOut, &wavehdr2, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveOutPrepareHeader failed\n");
			return -1;
		}
		waveOutWrite(hWaveOut, &wavehdr2, sizeof(WAVEHDR));

		// Get buffer three
		if ( waveInPrepareHeader(hwavein, &wavehdr3, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveInPrepareHeader failed\n");
			return -1;
		}
	
		if ( waveInAddBuffer(hwavein, &wavehdr3, length) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}
	
		waveInStart(hwavein);
		Sleep( 1000 * length / (wformat.nSamplesPerSec * wformat.nChannels * (wformat.nSamplesPerSec / 8)) );
		while ( (wavehdr3.dwFlags & WHDR_DONE) == 0)
			Sleep(10);
		waveInUnprepareHeader(hwavein, &wavehdr3, sizeof(WAVEHDR));
	
		waveOutUnprepareHeader(hWaveOut, &wavehdr2, sizeof(WAVEHDR));

		ProcessBuffer((short int *)data3, length / 2);

		if (waveOutPrepareHeader(hWaveOut, &wavehdr3, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveOutPrepareHeader failed\n");
			return -1;
		}
		waveOutWrite(hWaveOut, &wavehdr3, sizeof(WAVEHDR));

	}


//	waveInReset(hwavein);
//	waveInUnprepareHeader(hwavein, &wavehdr, sizeof(WAVEHDR));
//	waveInClose(hwavein);
	return 0;
}







int main(void)
{
	stream_wave();
	return 0;
}


