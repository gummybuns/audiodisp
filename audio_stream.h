#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "audio_ctrl.h"

// is it always a u_char? what if the precision is different?
// what if we are streaming ac3 or linear_se
typedef struct audiobuffer {
	int size;
	u_char *data;
} audio_buffer_t;

typedef struct audiostream {
	int milliseconds;
	int samples_streamed;
	int buffer_count;
	int channels;
	int sample_size;
	u_int encoding;
	audio_buffer_t **buffers;
} audio_stream_t;

int build_stream(
    int milliseconds,
    int channels,
    int sample_rate,
    int buffer_size,
    u_int encoding,
    audio_stream_t *stream
);
int stream(audio_ctrl_t ctrl, audio_stream_t *stream);
int clean_buffers(audio_stream_t *stream);
void print_stream(audio_stream_t stream);
#endif
