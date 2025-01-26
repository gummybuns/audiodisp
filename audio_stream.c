#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/audioio.h>
#include <unistd.h>

#include "audio_stream.h"
#include "audio_ctrl.h"

void print_stream(audio_stream_t stream)
{
	const char *encoding;

	encoding = get_encoding(stream.encoding);
	printf(
	    "STREAM\n"
	    "\tmilliseconds:\t\t%d\n"
	    "\tsamples_streamed:\t%d\n"
	    "\tchannels:\t\t%d\n"
	    "\tencoding:\t\t%s\n"
	    "\tprecision:\t\t%d\n"
	    "\tbuffer_count:\t\t%d\n",
	    stream.milliseconds,
	    stream.samples_streamed,
		stream.channels,
		encoding,
		stream.precision,
	    stream.buffer_count
	);
	printf("\tBUFFERS\n");
	for(int i = 0; i < stream.buffer_count; i++) {
		printf("\t\tbuffer[%d]\n", i);
		printf("\t\tsize: %d\n", stream.buffers[i]->size);
		printf("\t\tprecision: %d\n", stream.buffers[i]->precision);
	}
}
int build_stream_from_ctrl(audio_ctrl_t ctrl, int ms, audio_stream_t *stream)
{
	return build_stream(
	    ms,
	    ctrl.config.channels,
	    ctrl.config.sample_rate,
	    ctrl.config.buffer_size,
	    ctrl.config.precision,
	    ctrl.config.encoding,
	    stream
	);
}

int build_stream(
    int milliseconds,
    int channels,
    int sample_rate,
    int buffer_size,
    int precision,
    u_int encoding,
    audio_stream_t *stream
)
{
	int i;

	// TODO - i think this is right but need to spend some more thought on it
	//
	//   milliseconds = 2000
	//   channels = 2
	//   sample_rate = 1000. That means I get 1000 samples per second
	//   buffer_size = 1000
	//   precision = 16
	//
	// I need 2000 samples _per_ channel (sample_rate * milliseconds / 1000). 
	// Therefore I need 4000 samples total (samples_needed)
	//
	// each sample is 2 bytes (16 bits - bytes_per_sample = precision / 8),
	// 
	// and one buffer can only support 1000 bytes total (buffer_size).
	// that means there are 500 samples per buffer. (buffer_size / bytes_per_sample)
	// that means i need 8 buffers (samples_needed / samples_per_buffer)
	float samples_needed = ceilf(
	   (float) milliseconds / 1000 * sample_rate * channels
	);
	int bytes_per_sample = precision / STREAM_BYTE_SIZE;
	float samples_per_buffer = ceilf(buffer_size / bytes_per_sample);
	float buffers_needed = ceilf(samples_needed / samples_per_buffer);

	stream->milliseconds = milliseconds;
	stream->channels = channels;
	stream->encoding = encoding;
	stream->buffers = malloc(buffers_needed * sizeof(audio_buffer_t *));
	stream->total_samples = samples_needed;
	stream->precision = precision;
	stream->buffer_count = 0;
	stream->total_size = 0;
	stream->samples_streamed = 0;

	i = samples_needed;
	while (i > 0) {
		audio_buffer_t *buffer = malloc(sizeof(audio_buffer_t));
		// the size of the buffer is samples_per_buffer or whats left
		int size = (int) fminf((float) i, samples_per_buffer);
		buffer->size = size;
		buffer->precision = precision;
		buffer->samples = size / bytes_per_sample;
		buffer->data = malloc(size);
		stream->buffers[stream->buffer_count] = buffer;
		stream->buffer_count++;
		stream->total_size += size;
		i = i - size;
	}

	return 0;
}

int stream(audio_ctrl_t ctrl, audio_stream_t *stream)
{
	int io_count = 0;

	// TODO - i think i need validations here to make sure that the encodings
	// match? what else??

	for (int i = 0; i < stream->buffer_count; i++) {
		audio_buffer_t *buffer = stream->buffers[i];

		if (ctrl.mode == AUMODE_RECORD) {
			io_count = read(ctrl.fd, buffer->data, buffer->size);
		} else {
			io_count = write(ctrl.fd, buffer->data, buffer->size);
		}

		if (io_count < 0) {
			printf("Failed to stream\n");
			return -1;
		}
		stream->samples_streamed += buffer->size;
	}
}

int clean_buffers(audio_stream_t *stream)
{
	for (int i = 0; i < stream->buffer_count; i++) {
		free(stream->buffers[i]->data);
		free(stream->buffers[i]);
	}
	free(stream->buffers);

	return 0;
}

u_char *flatten_stream(audio_stream_t *stream)
{
	u_char *flattened = malloc(stream->total_size);
	int s = 0;
	for (int i = 0; i < stream->buffer_count; i++) {
		audio_buffer_t *buffer = stream->buffers[i];
		for (int j = 0; j < buffer->size; j++) {
			flattened[s] = buffer->data[j];
			s++;
		}
	}

	return flattened;
}
