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
	    "\tbuffer_count:\t\t%d\n",
	    stream.milliseconds,
	    stream.samples_streamed,
		stream.channels,
		encoding,
	    stream.buffer_count
	);
}
int build_stream(
    int milliseconds,
    int channels,
    int sample_rate,
    int buffer_size,
    u_int encoding,
    audio_stream_t *stream
)
{
	float samples_needed;
	int i;

	samples_needed = ceilf(
	   (float) milliseconds / 1000 * sample_rate * channels 
	);

	stream->milliseconds = milliseconds;
	stream->samples_streamed = 0;
	stream->channels = channels;
	stream->encoding = encoding;
	stream->buffer_count = 0;
	stream->buffers = malloc(stream->buffer_count * sizeof(audio_buffer_t *));
	stream->sample_size = samples_needed;

	i = samples_needed;
	while (i > 0) {
		audio_buffer_t *buffer = malloc(sizeof(audio_buffer_t));
		int size = (int) fminf((float) i, (float) buffer_size);
		buffer->size = size;
		buffer->data = malloc(sizeof(u_char) * size);
		stream->buffers[stream->buffer_count] = buffer;
		stream->buffer_count++;
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
