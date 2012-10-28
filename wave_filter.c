#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#define MY_PI 3.14159265359f
#define N 16
#define M 7

#define NUM_SAMPLES (44100 * 30)

float h[M] = {	0.0567f, 0.1560f, 0.2394f, 0.2719f, 0.2394f, 0.1560f, 0.0567f };


typedef struct
{
	short	format;
	short	channels;
	int	sample_rate;
	int	avg_sample_rate;
	short	align;
	short	sample_size;
} wave_t;

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


void generate_samples(float *x, int offset, int n)
{
	int i;

	for(i = 1 + offset; i <= n + offset; i++)
	{
		float t = i * (2 * MY_PI / 8000);
		x[i - offset - 1] = (float)sin(2.0f * MY_PI * 800 * t) / 2.0f + (float)sin(2.0f * MY_PI * 1400 * t) / 2.0f;

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
	Sleep( 1000 * length / (format->sample_rate * format->channels * (format->sample_size / 8)) + 1);
}

int main(void)
{
	complex_t		omega[(N-1)*(N-1) + 1];
	complex_t		m[N*N];
	int			i, j;
	wave_t			wave;
	unsigned short int	x_pcm1[30];
	unsigned short int	x_pcm2[30];
	unsigned short int *x_pcm = x_pcm1;
	unsigned short int *x_old = x_pcm2;
	unsigned short int *temp;
	float			x[30] = {0.0f};
	float			y[36] = {0.0f};

	wave.format = 1;
	wave.channels = 1;
	wave.sample_rate = 44100;
	wave.avg_sample_rate = 44100 * 1;
	wave.align = 2;
	wave.sample_size = 16;


	printf("calculating omega\n");
	calc_omega(omega, N);

	printf("filling matrix\n");
	fill_matrix(m, omega, N);


	printf("generating samples\n");

	printf("format\t\t%d\nchannels\t%d\nsampleRate\t%d\nsampleSize\t%d\n", wave.format, wave.channels, wave.sample_rate, wave.sample_size);
	for(i = 0; i < NUM_SAMPLES && NUM_SAMPLES - i > 30; i += 30)
	{
		generate_samples(&x[0], i, 30);


//		printf("filter %d\n", i);
		filter(&y[0], &y[30], &x[0], &h[0], &m[0]);
		for(j = 0; j < 30; j++)
		{
			//float [-1.0,1.0] -> pcm [0,65535]
			x_pcm[j] = (unsigned short int)(y[j] * 32767 + 32767);
		}
		play_wave(&wave, (char *)x_pcm, 30);
		temp = x_pcm;
		x_pcm = x_old;
		x_old = temp;

	}
	
	return 0;
}