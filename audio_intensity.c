#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <err.h>
#include <sys/audioio.h>
#include <math.h>
#include <kissfft/kiss_fft.h>

#include "audio_ctrl.h"
#include "audio_stream.h"

#define MU 255
#define MU_BIAS 33
#define MAX_LINEAR 32767
#define SIGN_MASK 0x80
#define EXCESS_127 0x7F

// numbers get MORE negative as the amplitude increases
float calculate_rms(signed char *data, int start, int end) {
	float sum = 0.0;
	int length = end - start;

	for (int i = start; i < (start + length); i++) {
		sum += (float) data[i] * (float) data[i];
	}

	return sqrtf((float) sum / (float) length);
}

int main(int argc, char *argv[])
{
	// drawing stuff
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	float percent = 0.0;

	// recording stuff
	char *recording_audio_path;
	int s_index = 0;
	audio_ctrl_t record_ctrl;
	audio_stream_t stream1, stream2;
	audio_stream_t streams[] = {stream1, stream2};
	audio_stream_t cur_stream;

	if (argc <= 1) {
		err(1, "Specify a recording audio device");
	}

	recording_audio_path = argv[1];
	build_audio_ctrl(&record_ctrl, recording_audio_path, AUMODE_RECORD);
	build_stream(
	    250,
	    record_ctrl.config.channels,
	    record_ctrl.config.sample_rate,
	    record_ctrl.config.buffer_size,
	    record_ctrl.config.encoding,
	    &streams[0]
	);
	build_stream(
	    250,
	    record_ctrl.config.channels,
	    record_ctrl.config.sample_rate,
	    record_ctrl.config.buffer_size,
	    record_ctrl.config.encoding,
		&streams[1]
	);
	
	initscr();
	noecho();
	getmaxyx(stdscr, row, col);
	y_padding = col / 10; 
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;

	printw("Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0\%");
	mvprintw(3 + x_padding, bar_start + bar_distance/2, "50\%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100\%");
	refresh();

	while(1) {
		s_index ^= 1;
		cur_stream = streams[s_index];
		stream(record_ctrl, &cur_stream);

		int size = 0;
		for (int i = 0; i < cur_stream.buffer_count; i++) {
			size += cur_stream.buffers[i]->size;
		}
		// it would be nice to have the full size as part of the stream
		signed char *full_sample = malloc(sizeof(u_char)*size);
		int li = 0;
		for (int i = 0; i < cur_stream.buffer_count; i++) {
			audio_buffer_t *buffer = cur_stream.buffers[i];

			for (int j = 0; j < buffer->size; j++) {
				u_char s = buffer->data[j];
				full_sample[li] = (signed char)(s > 127 ? s - 256 : s);
				li++;
			}
		}

		int chunk_size = 2000;
		int pp = 0;
		for (int i = 0; i < size; i += chunk_size) {
			int cz = (int) fmin((double) chunk_size, (double) size - i);
			float rms = calculate_rms(full_sample, i, i+cz);	
			percent = (float) rms / 127 * 100;
			if (percent >= 0) {
				draw_length = bar_distance * (percent / (float) 100.0);
				mvhline(2 + x_padding, bar_start, ' ', bar_distance);
				mvhline(2 + x_padding, bar_start, '=', draw_length);
				move(1, 0);
				refresh(); // do i need to refresh?
				pp++;
			} else {
				endwin();
				printf("size: %d, rms: %f, chunk_size: %d, start: %d end: %d\n", size, rms, chunk_size, i, i+cz);
				printf("pp = %d\n", pp);
				return -1;
			}
		}

		free(full_sample);
	}
	getch();
	endwin();

	// TODO clean buffers
	return 0;
}
