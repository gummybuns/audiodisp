#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "audio_ctrl.h"

#define STREAM_BYTE_SIZE 8

typedef struct audio_buffer_t {
	int samples;		/* number of samples in the buffer */
	int size;			/* total memory size of buffer */
	int precision;		/* number of bits per sample */
	u_char *data;		/* the buffer of data */
} audio_buffer_t;

typedef struct audios_buffer_t {
	int buffer_count;			/* number of buffers needed */
	int channels;				/* number of channels on the audio device */
	int milliseconds;			/* duration of the stream */
	int precision;				/* the number of bits per sample */
	int samples_streamed;		/* running count. i wanna delete */
	int total_size;				/* total memory size of all buffers */
	int total_samples;			/* total number of samples across all buffers */
	unsigned int encoding;		/* the encoding of the audio device */
	audio_buffer_t **buffers;	/* the buffers that hold the data */
} audio_stream_t;

typedef struct circular_list_t {
	int size;					/* the number of streams */
	audio_stream_t *streams;	/* the array of streams */
	int start;					/* the index of the first item in the list */
} circular_list_t;

int build_stream(int milliseconds, int channels, int sample_rate,
                 int buffer_size, int precision, u_int encoding,
                 audio_stream_t * stream);

int build_stream_from_ctrl(audio_ctrl_t ctrl, int ms, audio_stream_t * stream);
int stream(audio_ctrl_t ctrl, audio_stream_t * stream);
int clean_buffers(audio_stream_t * stream);
void print_stream(audio_stream_t stream);
u_char *flatten_stream(audio_stream_t * stream);
#endif
