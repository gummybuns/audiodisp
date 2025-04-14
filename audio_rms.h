#ifndef AUDIO_RMS_H
#define AUDIO_RMS_H

#define RMS_ERR_UNKNOWN_PRECISION 201

float calc_rms(void *data, u_int precision, u_int length);
float calc_rms_percent(float rms, u_int precision);

#endif
