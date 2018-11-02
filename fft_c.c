#include <stdio.h>
#include <math.h>

#define M_PI 3.14159265358979323846f

typedef struct
{
	float real;
	float imag;
} complex_t;


void fft_c(int num, complex_t *x, complex_t *w)
{
	complex_t u, temp, tm;
	complex_t *w_ptr;

	int i, j, k, length, w_index;

	// start fft
	w_index = 1;
	for (length = num / 2; length > 0; length /= 2)
	{
		w_ptr = w;
		for (j = 0; j < length; j++)
		{
			u = *w_ptr;
			for (i = j; i < num; i = i + 2 * length)
			{
				temp.real = x[i].real + x[i + length].real;
				temp.imag = x[i].imag + x[i + length].imag;
				tm.real = x[i].real - x[i + length].real;
				tm.imag = x[i].imag - x[i + length].imag;
				x[i + length].real = tm.real * u.real - tm.imag * u.imag;
				x[i + length].imag = tm.real * u.imag + tm.imag * u.real;
				x[i] = temp;

			}
			w_ptr = w_ptr + w_index;
		}
		w_index = 2 * w_index;
	}

	// rearrange data by bit reversing
	j = 0;
	for (i = 1; i < (num - 1); i++)
	{
		k = num / 2;
		while (k <= j)
		{
			j -= k;
			k /= 2;
		}
		j += k;
		if (i < j)
		{
			temp = x[j];
			x[j] = x[i];
			x[i] = temp;
		}
	}
}

void init_w(int n, complex_t *W)
{
	int i;

	float a = 2.0f * M_PI / n;

	for (i = 0; i < n; i++) {
		W[i].real = (float)cos(-i*a);
		W[i].imag = (float)sin(-i*a);
	}
}


int main(int argc, char *argv[])
{
	complex_t x[1024];
	complex_t w[1024];


	for (int i = 0; i < 1024; i++)
	{
		init_w(1024, w);
		x[i].real = sinf(i * 0.01f);
		x[i].imag = 0.0f;
	}

	fft_c(1024, x, w);

	for (int i = 0; i < 1024; i++)
	{
		// output matches matlab, but off by order of magnitude, divide by 100 to match
		printf("%f %f\r\n", x[i].real / 100.0f, x[i].imag / 100.0f);
	}

	return 0;
}
