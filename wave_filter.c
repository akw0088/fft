#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#define MY_PI 3.14159265359f
#define N 16
#define M 7
#define FREQUENCY 8000
#define NUM_SAMPLES (FREQUENCY * 10)


//least squares low pass, pass <1khz stop >2khz
float h[M] = {	0.0498f, 0.0516f, 0.0528f, 0.0531f, 0.0528f, 0.0516f, 0.0498f };


typedef struct
{
	short	format;
	short	channels;
	int	sample_rate;
	int	avg_sample_rate;
	short	align;
	short	sample_size;
} wave_t;
void play_wave(wave_t *format, char *data, int length);


typedef struct
{
	float real;
	float imag;
} complex_t;


void complex_multiply(complex_t *result, const complex_t *a, const complex_t *b)
{
	complex_t temp;

	temp.real = a->real * b->real - (a->imag * b->imag);
	temp.imag = a->real * b->imag + (a->imag * b->real);
	result->real = temp.real;
	result->imag = temp.imag;
}

void complex_power(complex_t *result, const complex_t *value, const int power)
{
//	(a+bi) (a+bi)
	complex_t	temp;
	int i;

	if (power < 0 || power == 1)
	{
		result->real = value->real;
		result->imag = value->imag;
		return;
	}

	if (power == 0)
	{
		result->real = 1.0f;
		result->imag = 0.0f;
		return;
	}

	result->real = value->real;
	result->imag = value->imag;

	for(i = 1; i < power; i++)
	{
		temp.real = result->real * value->real - (result->imag * value->imag);
		temp.imag = result->real * value->imag + (result->imag * value->real);
		result->real = temp.real;
		result->imag = temp.imag;
	}

}

void calc_omega(complex_t *omega, int n)
{
	int i;
	int size = (n-1) * (n-1) + 1;
	complex_t value;

	value.real = (float)cos( -2.0 * MY_PI / n);
	value.imag = (float)sin( -2.0 * MY_PI / n);

	for(i = 0; i < size; i++)
	{
		complex_power(&omega[i], &value, i);
		//printf("omega[%d] = (%2.2f, %2.2f)\n", i, omega[i].real, omega[i].imag);
	}
}

void fill_matrix(complex_t *m, complex_t *omega, int n)
{
	int i, j;

	for(j = 0; j < n; j++)
	{
		for(i = 0; i < n; i++)
		{
			m[i + j * n].real = omega[i*j].real;
			m[i + j * n].imag = omega[i*j].imag;

			//printf("matrix[%d][%d] = (%2.2f, %2.2f)\n", j, i, omega[i*j].real, omega[i*j].imag);
		}
	}
}

void dial_digit(short int *s_pcm, char digit, float time)
{
	int		f1 = 350;
	int		f2 = 440;
	float	t = 0;
	wave_t	wave;
	int i;

	wave.format = 1;
	wave.channels = 1;
	wave.sample_rate = 8000;
	wave.avg_sample_rate = 8000 * 1;
	wave.align = 2;
	wave.sample_size = 16;

    if (digit == '1')
	{
        f1 = 1209;
        f2 = 697;
    }
    if (digit == '2')
	{
        f1 = 1336;
        f2 = 697;
    }
    if (digit == '3')
	{
        f1 = 1447;
        f2 = 697;
    }
    if (digit == '4')
	{
        f1 = 1209;
        f2 = 770;
    }
    if (digit == '5')
	{
        f1 = 1336;
        f2 = 770;
    }
    if (digit == '6')
	{
        f1 = 1447;
        f2 = 770;
    }
    if (digit == '7')
	{
        f1 = 1209;
        f2 = 852;
    }
    if (digit == '8')
	{
        f1 = 1336;
        f2 = 852;
    }
    if (digit == '9')
	{
        f1 = 1447;
        f2 = 852;
    }
    if (digit == '0')
	{
        f1 = 1209;
        f2 = 941;
	}
    if (digit == '#')
	{
        f1 = 1447;
        f2 = 941;
	}

    for (i = 0; i < 8000 * time; i++)
	{
        s_pcm[i] = (0.5 * sin(2 * 3.14159 * f1 * t) + 0.5 * sin (2 * 3.14159 * f2 * t)) * 32767;
        t = t + 0.001;
	}
}

void dial_num(short int *s_pcm, float dial_time, char *phone_num)
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
            dial_digit(&s_pcm[(int)(i * 8000 * dial_time)], phone_num[i], dial_time);
		}
	}

}


void generate_samples(float *x, int offset, int n)
{
	int i;

	for(i = 1 + offset; i <= n + offset; i++)
	{
		float t = i * (2 * MY_PI / FREQUENCY);
		x[i - offset - 1] = 0.1f * (float)sin(2.0f * MY_PI * 440.0 * t) + 0.1f * (float)sin(2.0f * MY_PI * 350.0 * t);

//		printf("x[i] = %f\n", x[i-1]);
	}
}

void matrix_multiply(complex_t *result, const complex_t *matrix, const complex_t *input, const int size)
{
	int i, j;

	for(j = 0; j < size; j++)
	{
		result[j].real = 0;
		result[j].imag = 0;
		for(i = 0; i < size; i++)
		{
			// complex multiplication (3 + 2i)(1 + 4i) = 3 + 12i + 2i + 8i^2 
			result[j].real += input[i].real * matrix[i + j * size].real - (input[i].imag * matrix[i + j * size].imag);
			result[j].imag += input[i].real * matrix[i + j * size].imag + (input[i].imag * matrix[i + j * size].real);
		}
	}
}

void imatrix_multiply(complex_t *result, const complex_t *matrix, const complex_t *input, const int size)
{
	int i, j;

	for(j = 0; j < size; j++)
	{
		result[j].real = 0;
		result[j].imag = 0;
		for(i = 0; i < size; i++)
		{
			// complex multiplication (3 + 2i)(1 + 4i) = 3 + 12i + 2i + 8i^2 
			result[j].imag += input[i].imag * matrix[i + j * size].real - (input[i].real * matrix[i + j * size].imag);
			result[j].real += input[i].imag * matrix[i + j * size].imag + (input[i].real * matrix[i + j * size].real);
		}
		result[j].real /= size;
		result[j].imag /= size;
	}
}


void filter(float *y, const float*overlap, const float *x, const float *h, const complex_t *m)
{
	complex_t x1[16], x2[16], x3[16];
	complex_t y1[16], y2[16], y3[16];
	complex_t h_complex[16];
	complex_t impulse[16];
	int i;

	memset(&x1, 0, sizeof(complex_t) * 16);
	memset(&x2, 0, sizeof(complex_t) * 16);
	memset(&x3, 0, sizeof(complex_t) * 16);
	memset(&y1, 0, sizeof(complex_t) * 16);
	memset(&y2, 0, sizeof(complex_t) * 16);
	memset(&y3, 0, sizeof(complex_t) * 16);
	memset(&h_complex, 0, sizeof(complex_t) * 16);
	memset(&impulse, 0, sizeof(complex_t) * 16);


	// Get 3 buffers
	for(i = 0; i < 30; i++)
	{
		if (i < 10)
			x1[i].real = x[i];
		if (i >= 10 && i < 20)
			x2[i-10].real = x[i];
		if (i >= 20 && i < 30)
			x3[i-20].real = x[i];
	}

	for(i = 0; i < M; i++)
	{
		h_complex[i].real = h[i];
	}


	matrix_multiply(y1, m, x1, N);
	matrix_multiply(y2, m, x2, N);
	matrix_multiply(y3, m, x3, N);
	matrix_multiply(impulse, m, h_complex, N);


	for(i = 0; i < N; i++)
	{
		complex_multiply(&y1[i], &y1[i], &impulse[i]);
		complex_multiply(&y2[i], &y2[i], &impulse[i]);
		complex_multiply(&y3[i], &y3[i], &impulse[i]);
	}



	imatrix_multiply(x1, m, y1, N);
	imatrix_multiply(x2, m, y2, N);
	imatrix_multiply(x3, m, y3, N);


	for(i = 0; i < 36; i++)
	{
		if ( i < 6 )
			y[i] = x1[i].real + overlap[i];
		else if ( i < 10)
			y[i] = x1[i].real;
		else if (i < 16)
			y[i] = x1[i].real + x2[i-10].real;
		else if ( i < 20)
			y[i] = x2[i-10].real;
		else if (i < 26)
			y[i] = x2[i-10].real + x3[i-20].real;
		else
			y[i] = x3[i-20].real;
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
	Sleep( 1000 * length / (format->sample_rate * format->channels * (format->sample_size / 8)));
	waveOutClose(hWaveOut);
}

short int	x_pcm[NUM_SAMPLES];
short int	y_pcm[NUM_SAMPLES];
float			x[NUM_SAMPLES] = {0.0f};
float			y[NUM_SAMPLES + 6] = {0.0f};


int main(void)
{
	complex_t		omega[(N-1)*(N-1) + 1];
	complex_t		m[N*N];
	int				i;
	wave_t			wave;
	char phone_num[] = "19408916875";
	short int *s_pcm;
	float dial_time = 0.8;


	wave.format = 1;
	wave.channels = 1;
	wave.sample_rate = FREQUENCY;
	wave.avg_sample_rate = FREQUENCY * 1;
	wave.align = 2;
	wave.sample_size = 16;

//	s_pcm = (short int *)malloc(strlen(phone_num) * 8000 * dial_time);
//	if (s_pcm == NULL)
//		return;

//	dial_num(s_pcm, dial_time, phone_num);

	printf("calculating omega\n");
	calc_omega(omega, N);

	printf("filling matrix\n");
	fill_matrix(m, omega, N);


	printf("generating samples\n");
	generate_samples(&x[0], 0, NUM_SAMPLES);

	for(i = 0; i < NUM_SAMPLES && NUM_SAMPLES - i > 30; i += 30)
	{
		filter(&y[i], &y[i+30], &x[i], &h[0], &m[0]);
	}
	
	for(i = 0; i < NUM_SAMPLES; i++)
	{
		//float [-1.0,1.0] -> pcm [0,65535]
		x_pcm[i] = (short int)(x[i] * 32767);
		y_pcm[i] = (short int)(y[i] * 32767);
	}

	printf("format\t\t%d\nchannels\t%d\nsampleRate\t%d\nsampleSize\t%d\n", wave.format, wave.channels, wave.sample_rate, wave.sample_size);
	printf("Unfiltered\n");
	play_wave(&wave, (char *)x_pcm, NUM_SAMPLES);
	printf("Filtered\n");
	play_wave(&wave, (char *)y_pcm, NUM_SAMPLES);
	
	return 0;
}