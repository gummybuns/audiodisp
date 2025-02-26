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
		sum += (float)data[i] * (float)data[i];
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
		mvprintw(2, 0, "%f\n", rms);
		return rms / SCHAR_MAX * 100;
	case 16:
		rms = rms16((short *)data, start, end);
		mvprintw(2, 0, "%f\n", rms);
		return rms / SHRT_MAX * 100;
	case 32:
		rms = rms32((float *)data, start, end);
		mvprintw(2, 0, "%f\n", rms);
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
	if (keypress == 'I')
		return DISPLAY_INFO;
	else if (keypress == 'P')
		return DISPLAY_PLAYBACK;
	else if (keypress == 'R')
		return DISPLAY_RECORD;
	else if (keypress == 'E')
		return DISPLAY_ENCODING;
	else if (keypress == 'Q')
		return DISPLAY_EXIT;
	else
		return 0;
}

/*
 * Display information about the audio controlers + streams
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
char
display_info(audio_ctrl_t ctrl, circular_list_t *stream_list)
{
	char keypress;
	char option;

	move(0, 0);
	nodelay(stdscr, FALSE);
	print_ctrl(ctrl);
	print_stream(stream_list->streams[0]);
	for (;;) {
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_INFO) {
			return option;
		}
	}
}

/*
 * Display a form for the user to change the encodings for both the recording
 * controller and play controller
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
char
display_encodings(audio_ctrl_t *rctrl, audio_ctrl_t *pctrl)
{
	char keypress;
	char option;
	int index, pmax, rmax;
	int res;
	audio_encoding_t encoding;

	nodelay(stdscr, FALSE);

	rmax = rctrl->encoding_options.total;
	pmax = pctrl->encoding_options.total;

	for (;;) {
		mvprintw(0, 0, "Enter a letter to update the encoding\n\n");
		printw("HEY - YOU BETTER CHOOSE THE SAME ENCODINGS FOR BOTH FOR NOW OR YOUR EARS WILL HURT\n\n");
		printw("Available Encodings for %s\n", rctrl->path);
		print_encodings(rctrl, 0);

		printw("\nAvailable Encodings for %s\n", pctrl->path);
		print_encodings(pctrl, rmax);

		refresh();
		keypress = (char)getch();
		option = check_options(keypress);

		if (option != 0 && option != DISPLAY_ENCODING)
			return option;

		if (
		    keypress >= ENC_OPTION_OFFSET
		    && keypress < rmax + ENC_OPTION_OFFSET
		    ) {
			printw("TRYING TO UPDATE ENCODING\n");
			index = keypress - ENC_OPTION_OFFSET;
			encoding = rctrl->encoding_options.encodings[index];
			rctrl->config.precision = (u_int)encoding.precision;
			rctrl->config.encoding = (u_int)encoding.encoding;

			/* TODO error handling for set encoding */
			res = update_ctrl(rctrl);
			printw("RESULT IS %d\n", res);
		}
		if (
		    keypress >= ENC_OPTION_OFFSET + rmax
		    && keypress < rmax + pmax + ENC_OPTION_OFFSET
		    ) {
			printw("TRYING TO UPDATE ENCODING\n");
			index = keypress - ENC_OPTION_OFFSET - rmax;
			encoding = pctrl->encoding_options.encodings[index];
			pctrl->config.precision = (u_int)encoding.precision;
			pctrl->config.encoding = (u_int)encoding.encoding;

			/* TODO error handling for set encoding */
			res = update_ctrl(pctrl);
			printw("RESULT IS %d\n", res);
		}
	}
}

/*
 * Displays a screen for a user to play back the recorded audio
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
char
display_playback(audio_ctrl_t play_ctrl, circular_list_t *stream_list)
{
	char keypress;
	char option;
	u_int i;
	u_int pause;
	audio_stream_t audio_stream;

	mvprintw(0, 0, "Playing last 5 seconds of audio\n");
	printw("SPACE: Pause/Resume\n");
	printw("ENTER: Restart\n");

	nodelay(stdscr, TRUE);

	pause = 0;
	i = stream_list->start;

	do {
		audio_stream = stream_list->streams[i];
		stream(play_ctrl, &audio_stream);
		i = (i + 1) % stream_list->size;
	} while(i != stream_list->start);

	for (;;) {
		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_PLAYBACK) {
			return option;
		}

		refresh();

		if (keypress == SPACE) {
			pause ^= 1;
			play_ctrl.config.pause = pause;
			if (update_ctrl(&play_ctrl) != 0) {
				/* handle failures */
				printw("Failed to update pause state\n");
			}
			print_ctrl(play_ctrl);
			move(3,0);
		} else if (keypress == ENTER) {
			i = stream_list->start;
			// todo - need to flush and restart everything
		}
	}
}

/*
 * Displays a screen to record audio and show the intensity of the audio
 * in discrete chunks based on each stream in the stream_list. Saves the
 * recorded data to the stream_list so it can be played back later
 *
 * Wait for a user to press one of navigation options. Returns the pressed
 * navigation option so the main routine can render the next screen
 */
char
display_intensity(audio_ctrl_t record_ctrl, circular_list_t *stream_list)
{
	char option;
	char keypress;
	u_int chunk_size;
	u_int count, i, si, z;
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	float percent;
	audio_stream_t audio_stream;
	void *full_sample;


	getmaxyx(stdscr, row, col);
	y_padding = col / 10;
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;

	mvprintw(0, 0, "Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0%%");
	mvprintw(3 + x_padding, bar_start + bar_distance / 2, "50%%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100%%");
	move(2 * x_padding, 0);
	refresh();

	nodelay(stdscr, TRUE);

	count = 0;
	si = 0;
	for (;;) {
		audio_stream = stream_list->streams[si];
		stream(record_ctrl, &audio_stream);
		full_sample = flatten_stream(&audio_stream);

		chunk_size = audio_stream.total_size;
		for (i = 0; i < audio_stream.total_samples; i += chunk_size){
			z = (u_int)fmin(
			    (double)chunk_size,
			    (double)audio_stream.total_size - i
			);

			percent = calculate_rms_percent(
			    full_sample,
			    audio_stream.precision,
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
				printw("%f\n", percent);
				refresh();
			} else {
				// TODO: error handling should be better
				//endwin();
				//free(full_sample);
				//printf("Unsupported codec\n");
				//exit(-1);
			}
		}

		keypress = (char)getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_RECORD)
			return option;

		/*
		 * TODO i dont like that this loop is responsible for maintaining the
		 * index / start. that should be the circular_lists job
		 */
		count++;
		si = count % stream_list->size;
		stream_list->start = count > stream_list->size
		    ? (si + 1) % stream_list->size
		    : 0;

		mvhline(18, 0, ' ', 50);
		mvprintw(
		    18,
		    0,
		    "count %d, si %d, start %d",
		    count,
		    si,
		    stream_list->start
		);
		move(19, 0);

		free(full_sample);
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
	printw("R: RECORD / P: PLAY / I: INFO / E: ENCODINGS / Q: QUIT");
	refresh();
}
