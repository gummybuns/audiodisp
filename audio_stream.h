#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "audio_ctrl.h"

#define STREAM_BYTE_SIZE 8

typedef struct audiobuffer {
	/**
	 * the total memory size of the buffer
	 */
	int size;

	/**
     * the number of samples in this buffer
	 * this is a calculation based off the size and precision
	 */
	int samples;

	/**
	 * the number of bits per sample
	 */
	int precision;

	/**
	 * the buffer of data
	 */
	u_char *data;
} audio_buffer_t;

typedef struct audiostream {
	/**
	 * the duration of the stream
	 */
	int milliseconds;

	/**
	 * the number of samples that have been streamed by running stream()
	 * used for debugging mostly. maybe i will get rid of it
	 */
	int samples_streamed;

	/**
	 * the number of buffers used to support a stream of the desired duration.
	 */
	int buffer_count;

	/**
	 * the number of channels on the audio device that we are streaming to
	 */
	int channels;

	/**
	 * the total size of all buffers
	 */
	 int total_size;

	/**
	 * the total number of samples across all buffers
	 */
	int total_samples;

	/**
	 * the number of bits per sample
	 */
	int precision;

	/**
	 * the encoding of the audio device we are streaming to
	 */
	u_int encoding;

	/**
	 * the array of buffers. When built using build_stream() each buffer will
	 * be at most buffer_size
	 */
	audio_buffer_t **buffers;
} audio_stream_t;

int build_stream(
    int milliseconds,
    int channels,
    int sample_rate,
    int buffer_size,
    int precision,
    u_int encoding,
    audio_stream_t *stream
);

int build_stream_from_ctrl(audio_ctrl_t ctrl, int ms, audio_stream_t *stream);
int stream(audio_ctrl_t ctrl, audio_stream_t *stream);
int clean_buffers(audio_stream_t *stream);
void print_stream(audio_stream_t stream);
u_char *flatten_stream(audio_stream_t *stream);
#endif
