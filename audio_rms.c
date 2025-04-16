#include <sys/audioio.h>

#include <math.h>

#include "audio_rms.h"

/*
 * Calculate the rms of char data
 */
static float
rms8(char *data, u_int length)
{
	u_int i;
	float sum;

	sum = 0.0;

	for (i = 0; i < length; i++) {
		sum += (float)data[i] * (float)data[i];
	}

	return sqrtf((float)sum / (float)length);
}

/*
 * Calculate the rms of short data
 */
static float
rms16(short *data, u_int length)
{
	u_int i;
	float sum;

	sum = 0.0;

	for (i = 0; i < length; i++) {
		sum += (float)data[i] * (float)data[i];
	}

	return sqrtf((float)sum / (float)length);
}

/*
 * Calculate the rms of float data
 */
static float
rms32(float *data, u_int length)
{

	u_int i;
	float sum;

	sum = 0.0;

	for (i = 0; i < length; i++) {
		sum += data[i] * data[i];
	}

	return sqrtf((float)sum / (float)length);
}

/*
 * Calculate the root mean square of the data buffer
 */
float
calc_rms(void *data, u_int precision, u_int length)
{
	switch (precision) {
	case 8:
		return rms8((char *)data, length);
	case 16:
		return rms16((short *)data, length);
	case 32:
		return rms32((float *)data, length);
	default:
		return -1;
	}
}

/*
 * Calculates the percentage of the rms value based on the precision
 */
float
calc_rms_percent(float rms, u_int precision)
{
	switch (precision) {
	case 8:
		return rms / SCHAR_MAX * 100;
	case 16:
		return rms / SHRT_MAX * 100;
	case 32:
		return rms / FLT_MAX * 100;
	default:
		return -1;
	}
}
