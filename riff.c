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


int main(void)
{
	char	*data;
	wave_t	*format;
	char	*pcmData;
	char	*end;
	int	length;

	data = get_file("test.wav");
	if (data == NULL)
	{
		printf("Couldnt open test.wav\n");
		return 0;
	}

	if ( check_format(data, "WAVE") )
	{
		printf("Not a wave file.\n");
		return 0;
	}

	end = data + 4 + *((int *)(data + 4));

	format = (wave_t *)find_chunk( data + 12, "fmt ", &length, end);
	pcmData = find_chunk( data + 12, "data", &length, end);

	printf("format\t\t%d\nchannels\t%d\nsampleRate\t%d\nsampleSize\t%d\n", format->format, format->channels, format->sample_rate, format->sample_size);
	play_wave(format, pcmData, length);
	return 0;
}


