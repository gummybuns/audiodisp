#include <curses.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>

#include "audio_displays.h"
#include "audio_ctrl.h"
#include "audio_stream.h"

#define SPACE 32
#define ENTER 10

/*
 * Calculate the rms of char data
 */
static float
rms8(char *data, u_int start, u_int end)
{
	u_int i, length;
	float sum;

	sum = 0.0;
	length = end - start;

	for (i = start; i < (start + length); i++){
		sum += (float)data[i] * (float)data[i];
	}

	return sqrtf((float)sum / (float)length);
}

/*
 * Calculate the rms of short data
 */
static float
rms16(short *data, u_int start, u_int end)
{
	u_int i, length;
	float sum;

	sum = 0.0;
	length = end - start;

	for (i = start; i < (start + length); i++){
		sum += (float)data[i] * (float)data[i];
	}

	return sqrtf((float)sum / (float)length);
}

/*
 * Calculate the rms of float data
 */
static float
rms32(float *data, u_int start, u_int end)
{

	u_int i, length;
	float sum;

	sum = 0.0;
	length = end - start;

	for (i = start; i < (start + length); i++){
		sum += data[i] * data[i];
	}

	return sqrtf((float)sum / (float)length);
}

/*
 * Calculate the rms of for a chunk of audio data
 */
static float
calculate_rms_percent(void *data, u_int precision, u_int start, u_int end)
{
	float rms;

	switch (precision) {
	case 8:
		rms = rms8((char *)data, start, end);
		return rms / SCHAR_MAX * 100;
	case 16:
		rms = rms16((short *)data, start, end);
		return rms / SHRT_MAX * 100;
	case 32:
		rms = rms32((float *)data, start, end);
		return rms / FLT_MAX * 100;
	default:
		endwin();
		return -1;
	}
}

/*
 * Check if the user pressed any of the navigation options
 */
static char
check_options(int keypress)
{
	if (keypress == 'I') {
		return DISPLAY_INFO;
	} else if (keypress == 'R') {
		return DISPLAY_RECORD;
	} else if (keypress == 'Q') {
		return DISPLAY_EXIT;
	} else {
		return 0;
	}
}

/*
 * Display information about the audio controlers + streams
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
char
display_info(audio_ctrl_t ctrl, audio_stream_t audio_stream)
{
	char keypress;
	char option;

	move(0, 0);
	nodelay(stdscr, FALSE);
	print_ctrl(ctrl);
	print_stream(audio_stream);
	for (;;) {
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_INFO) {
			return option;
		}
	}
}

/*
 * Displays a screen to record audio and show the intensity based on the
 * Root Mean Square
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
char
display_intensity(audio_ctrl_t record_ctrl, audio_stream_t *audio_stream)
{
	char option;
	char keypress;
	u_int chunk_size;
	u_int i, z;
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	float percent;
	void *full_sample;


	getmaxyx(stdscr, row, col);
	y_padding = col / 10;
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;

	mvprintw(0, col / 2 - 10, "Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0%%");
	mvprintw(3 + x_padding, bar_start + bar_distance / 2 - 2, "50%%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100%%");
	refresh();

	nodelay(stdscr, TRUE);

	for (;;) {
		stream(record_ctrl, audio_stream);
		full_sample = flatten_stream(audio_stream);

		chunk_size = audio_stream->total_size;
		for (i = 0; i < audio_stream->total_samples; i += chunk_size){
			z = (u_int)fmin(
			    (double)chunk_size,
			    (double)audio_stream->total_size - i
			);

			percent = calculate_rms_percent(
			    full_sample,
			    audio_stream->precision,
			    i,
			    i + z
			);
			if (percent >= 0) {
				draw_length = (int)(
				    (float)bar_distance * (percent / (float)100.0)
				);
				mvhline(2 + x_padding, bar_start, ' ', bar_distance);
				mvhline(2 + x_padding, bar_start, '=', draw_length);
				move(1, 0);
				refresh();
			} else {
				/* TODO: error handling should be better */
				endwin();
				free(full_sample);
				printf("Unsupported codec\n");
				exit(-1);
			}
		}

		free(full_sample);

		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_RECORD) {
			return option;
		}
	}
}

/*
 * Renders the nav options at the bottom of the screen for the user to see
 */
void
display_options(void)
{
	int row;
	row = getmaxy(stdscr);
	mvprintw(row - 1, 0, "OPTIONS: ");
	printw("R: RECORD / I: INFO / Q: QUIT");
	refresh();
}
