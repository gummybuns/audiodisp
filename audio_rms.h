#ifndef AUDIO_RMS_H
#define AUDIO_RMS_H

float calc_rms(void *data, u_int precision, u_int length);
float calc_rms_percent(float rms, u_int precision);

#endif
