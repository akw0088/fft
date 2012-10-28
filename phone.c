#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <linux/soundcard.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



#define MY_PI 3.14159265359f
#define N 16
#define M 7
#define SAMPLE_RATE 8000


typedef struct
{
	short	format;
	short	channels;
	int		sample_rate;
	int		avg_sample_rate;
	short	align;
	short	sample_size;
} wave_t;

int play_wave(wave_t *format, char *data, int length);

void dial_digit(short int *s_pcm, char digit, int tone_size)
{
	int		f1 = 350;
	int		f2 = 440;
	int		i;
	float tone;

	if (digit == '1')
	{
		f1 = 1209;
		f2 = 697;
	}
	else if (digit == '2')
	{
		f1 = 1336;
		f2 = 697;
	}
	else if (digit == '3')
	{
		f1 = 1447;
		f2 = 697;
	}
	else if (digit == '4')
	{
		f1 = 1209;
		f2 = 770;
	}
	else if (digit == '5')
	{
		f1 = 1336;
		f2 = 770;
	}
	else if (digit == '6')
	{
		f1 = 1447;
		f2 = 770;
	}
	else if (digit == '7')
	{
		f1 = 1209;
		f2 = 852;
	}
	else if (digit == '8')
	{
		f1 = 1336;
		f2 = 852;
	}
	else if (digit == '9')
	{
		f1 = 1447;
		f2 = 852;
	}
	else if (digit == '0')
	{
		f1 = 1209;
		f2 = 941;
	}
	else if (digit == '#')
	{
		f1 = 1447;
		f2 = 941;
	}
	else if (digit == '.')
	{
		f1 = 0;
		f2 = 0;
	}
	else if (digit == '_')
	{
		f1 = 350;
		f2 = 440;
	}
	else if (digit == 'b')
	{
		f1 = 480;
		f2 = 620;
	}

	for (i = 0; i < tone_size; i++)
	{
		float t = i * (2 * MY_PI / SAMPLE_RATE);
		
		tone = 0.5 * sin(2 * MY_PI * f1 * t) + 0.5 * sin(2 * MY_PI * f2 * t);
		s_pcm[i] = 32767 * tone;
	}
}

void dial_num(short int *s_pcm, char *phone_num, int tone_size)
{
	int i = 0;

	for (i = 0; i < strlen(phone_num); i++)
	{
		switch (phone_num[i])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '*':
		case '#':
		case '.':
		case '_':
			dial_digit(&s_pcm[i * tone_size], phone_num[i], tone_size);
			break;
		}
	}

}

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


#ifdef WIN32
int play_wave(wave_t *format, char *data, int length)
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
	Sleep( 1000 * length / (format->sample_rate * format->channels * (format->sample_size / 8)));
	waveOutClose(hWaveOut);
	return 0;
}
#else
int play_wave(wave_t *format, char *data, int length)
{
	int fd;
	int arg;
	int status;

	fd = open("/dev/dsp", O_RDWR);
	if (fd < 0)
	{
		perror("open of /dev/dsp failed");
		return -1;
	}

	/* set sampling parameters */
	arg = format->sample_size;
	status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
	if (status == -1)
	{
		perror("SOUND_PCM_WRITE_BITS ioctl failed");
		return -1;
	}
	if (arg != format->sample_size)
	{
		perror("unable to set sample size");
		return -1;
	}

	arg = format->channels;
	status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
	if (status == -1)
	{
		perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
		return -1;
	}
	if (arg != format->channels)
	{
		perror("unable to set number of channels");
		return -1;
	}

	arg = format->sample_rate;
	status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
	if (status == -1)
	{
		perror("SOUND_PCM_WRITE_WRITE ioctl failed");
		return -1;
	}

	status = write(fd, data, length);
	if (status != length)
	{
		perror("write failed");
		return -1;
	}

	// wait for playback to complete
	status = ioctl(fd, SOUND_PCM_SYNC, 0); 
	if (status == -1)
	{
		perror("SOUND_PCM_SYNC ioctl failed");
		return -1;
	}
}
#endif


int main(void)
{
	wave_t			wave;
	char phone_num[] = "___1.9.4.0.8.9.1.6.8.7.5";
	short int *s_pcm;
	float dial_time = 0.08f;
	int buffer_size = 2 * strlen(phone_num) * SAMPLE_RATE * dial_time;


	wave.format = 1;
	wave.channels = 1;
	wave.sample_rate = SAMPLE_RATE;
	wave.avg_sample_rate = SAMPLE_RATE * 1;
	wave.align = 2;
	wave.sample_size = 16;

	s_pcm = (short int *)malloc(buffer_size);
	if (s_pcm == NULL)
	{
		perror("malloc failed");
		return 0;
	}

	dial_num(s_pcm, phone_num, SAMPLE_RATE * dial_time);

	printf("format\t\t%d\nchannels\t%d\nsampleRate\t%d\nsampleSize\t%d\n", wave.format, wave.channels, wave.sample_rate, wave.sample_size);
	play_wave(&wave, (char *)s_pcm, buffer_size);
	free((void *)s_pcm);	
	return 0;
}
