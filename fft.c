#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MY_PI 3.14159265359f
#define N 16
#define M 7


float h[M] = {	0.0567, 0.1560, 0.2394, 0.2719, 0.2394, 0.1560, 0.0567 };

typedef struct
{
	float real;
	float imag;
} complex_t;


void complex_multiply(complex_t *result, complex_t *a, complex_t *b)
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

	value.real = cos( -2.0 * MY_PI / n);
	value.imag = sin( -2.0 * MY_PI / n);

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


void generate_samples(float *x, int n)
{
	int i;

	for(i = 1; i <= n; i++)
	{
		float t = i * (2 * MY_PI / 8000);
		x[i-1] = sin(2.0f * MY_PI * 800 * t) + sin(2.0f * MY_PI * 1400 * t);
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


void filter(float *x, float *h, complex_t *m)
{
	complex_t x1[36], x2[36], x3[36];
	complex_t y1[36], y2[36], y3[36];
	complex_t h_complex[16];
	complex_t impulse[16];
	int i;

	memset(&x1, 0, sizeof(complex_t) * 36);
	memset(&x2, 0, sizeof(complex_t) * 36);
	memset(&x3, 0, sizeof(complex_t) * 36);
	memset(&y1, 0, sizeof(complex_t) * 36);
	memset(&y2, 0, sizeof(complex_t) * 36);
	memset(&y3, 0, sizeof(complex_t) * 36);
	memset(&h_complex, 0, sizeof(complex_t) * 16);
	memset(&impulse, 0, sizeof(complex_t) * 16);


	// Get 3 buffers
	for(i = 0; i < 30; i++)
	{
		if (i < 10)
			x1[i].real = x[i];
		if (i >= 10 && i < 20)
			x2[i].real = x[i];
		if (i >= 20 && i < 30)
			x3[i].real = x[i];
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

	for(i = 0; i < N; i++)
	{
//		printf("x1[%d] = (%f, %f)\n", i,  x1[i].real, x1[i].imag);
//		printf("y1[%d] = (%f, %f)\n", i,  y1[i].real, y1[i].imag);
	}

	for(i = 0; i < 30; i++)
	{
		if ( i < 10)
			x[i] = x1[i].real;
		else if (i < 16)
			x[i] = x1[i].real + x2[i].real;
		else if ( i < 20)
			x[i] = x2[i].real;
		else if (i < 26)
			x[i] = x2[i].real + x3[i].real;
		else
			x[i] = x3[i].real;
	}
}


int main(void)
{
	complex_t	omega[(N-1)*(N-1)];
	complex_t	m[N*N];
	float		x[30];
	int		i;

	printf("calculating omega\n");
	calc_omega(omega, N);

	printf("filling matrix\n");
	fill_matrix(m, omega, N);


	printf("generating samples\n");
	generate_samples(&x[0], 30);

	printf("filtering\n");
	filter(&x[0], &h[0], &m[0]);
	for(i = 0; i < 30; i++)
	{
		printf("x[i] = %f\n", x[i]);
	}
	
	return 0;
}