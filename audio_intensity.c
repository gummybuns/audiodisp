#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <err.h>
#include <sys/audioio.h>
#include <math.h>
#include <kissfft/kiss_fft.h>
// there is also /usr/pkg/include/sndfile.h
// sndfile.h apparently can convert between the different data structures

#include "audio_ctrl.h"
#include "audio_stream.h"

#define STREAM_DURATION 250
#define ESC 27


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
	int keypress;
	float percent = 0.0;

	// recording stuff
	char *recording_audio_path;
	int s_index = 0;
	audio_ctrl_t record_ctrl;
	audio_stream_t stream1, stream2;

	if (argc <= 1) {
		err(1, "Specify a recording audio device");
	}

	recording_audio_path = argv[1];
	build_audio_ctrl(&record_ctrl, recording_audio_path, AUMODE_RECORD);
	build_stream_from_ctrl(record_ctrl, STREAM_DURATION, &stream1);
	
	initscr();
	raw();
	noecho();
	nodelay(stdscr, TRUE);
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
	move(2*x_padding, 0);
	refresh();

	// TODO i want to define a macro so i can have my print methods work
	// in either printf / printw
	//print_ctrl(record_ctrl);
	//print_stream(stream1);
	while(1) {
		keypress = getch();
		if (keypress == ESC) break;

		stream(record_ctrl, &stream1);

		// it would be nice to have the full size as part of the stream
		// TODO there needs to be something else done here, if the precision
		// is 16 for example, we want short instead of char...
		// i would like to have a method that copies the data over to the
		// flattened buffer
		signed char *full_sample = malloc(stream1.total_size);
		int li = 0;
		for (int i = 0; i < stream1.buffer_count; i++) {
			audio_buffer_t *buffer = stream1.buffers[i];

			for (int j = 0; j < buffer->size; j++) {
				u_char s = buffer->data[j];
				full_sample[li] = (signed char)(s > 127 ? s - 256 : s);
				li++;
			}
		}

		// TODO - i chose 2000 because that is the number of samples in
		// 250s of MU-LAW stream. This needs to be dynamically calculated
		// or i can just leave it at 2000, and we just get calculations
		int chunk_size = 2000;
		int pp = 0;
		for (int i = 0; i < stream1.total_size; i += chunk_size) {
			int z = (int) fmin(
			    (double) chunk_size,
			    (double) stream1.total_size - i
			);

			float rms = calculate_rms(full_sample, i, i+z);	
			percent = (float) rms / 127 * 100;
			if (percent >= 0) {
				draw_length = bar_distance * (percent / (float) 100.0);
				mvhline(2 + x_padding, bar_start, ' ', bar_distance); // is it better to clear the screen?
				mvhline(2 + x_padding, bar_start, '=', draw_length);
				move(1, 0);
				refresh(); // do i need to refresh?
				pp++;
			} else {
				// TODO think about what i want to happen if something goes wrong
				endwin();
				return -1;
			}
		}

		free(full_sample);
	}
	endwin();

	clean_buffers(&stream1);
	return 0;
}
