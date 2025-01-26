#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <err.h>
#include <sys/audioio.h>
#include <math.h>

// Possibly include fft to transform data to only include human audible spectrum
// maybe it can be used to eliminate a feedback loop so we can play the audio
// back that is recorded in real time
//#include <kissfft/kiss_fft.h>

#include "audio_ctrl.h"
#include "audio_stream.h"

#define STREAM_DURATION 250
#define ESC 27


float rms_mulaw(char *data, int start, int end) {
	float sum = 0.0;
	int length = end - start;

	for (int i = start; i < (start + length); i++) {
		sum += (float) data[i] * (float) data[i];
	}

	return sqrtf((float) sum / (float) length);
}

// TODO - i think based on whatever the encoding is i need a seaparate method
// to transform the data before i calculate the rms but im not entirely sure
// possibly use sndfile to transform for me? idk..
float calculate_rms_percent(u_char *data, int encoding, int start, int end) {
	switch(encoding) {
		case AUDIO_ENCODING_ULAW:
			return rms_mulaw((char *) data, start, end) / 127 * 100;
		default:
			return -1;
	}
}

int main(int argc, char *argv[])
{
	// drawing stuff
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	int keypress;

	// recording stuff
	char *recording_audio_path;
	int result;
	audio_ctrl_t record_ctrl;
	audio_stream_t stream1, stream2;

	if (argc <= 1) {
		err(1, "Specify a recording audio device");
	}

	recording_audio_path = argv[1];

	result = build_audio_ctrl(&record_ctrl, recording_audio_path, AUMODE_RECORD);
	if (result != 0) {
		err(result, "Failed to build audio controller");
	}

	result = build_stream_from_ctrl(record_ctrl, STREAM_DURATION, &stream1);
	if (result != 0) {
		err(result, "Failed to build audio stream");
	}

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
		u_char *full_sample = flatten_stream(&stream1);

		// TODO - i chose 2000 because that is the number of samples in
		// 250s of MU-LAW stream. This needs to be dynamically calculated
		// or i can just leave it at 2000, and we just get calculations
		//int chunk_size = 2000;
		int chunk_size = stream1.total_size;
		for (int i = 0; i < stream1.total_samples; i += chunk_size) {
			int z = (int) fmin(
			    (double) chunk_size,
			    (double) stream1.total_size - i
			);

			float percent = calculate_rms_percent(
			    full_sample,
			    stream1.encoding,
			    i,
			    i+z
			);	
			if (percent >= 0) {
				draw_length = bar_distance * (percent / (float) 100.0);
				mvhline(2 + x_padding, bar_start, ' ', bar_distance);
				mvhline(2 + x_padding, bar_start, '=', draw_length);
				move(1, 0);
				printw("%f\n", percent);
				refresh();
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
