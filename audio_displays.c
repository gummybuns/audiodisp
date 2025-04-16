#include <curses.h>
#include <err.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "audio_ctrl.h"
#include "audio_displays.h"
#include "audio_rms.h"
#include "audio_stream.h"

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
	int title_center;
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	float rms, percent;
	void *full_sample;

	getmaxyx(stdscr, row, col);
	y_padding = col / 10;
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;
	title_center = col / 2 - 10;

	mvprintw(0, title_center, "Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0%%");
	mvprintw(3 + x_padding, bar_start + bar_distance / 2 - 2, "50%%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100%%");
	refresh();

	nodelay(stdscr, TRUE);

	for (;;) {
		/* record the audio to the stream */
		stream(record_ctrl, audio_stream);
		full_sample = flatten_stream(audio_stream);

		/* calculate rms */
		rms = calc_rms(full_sample, audio_stream->precision,
		    audio_stream->total_samples);
		percent = calc_rms_percent(rms, audio_stream->precision);

		if (percent < 0) {
			endwin();
			free(full_sample);
			err(RMS_ERR_UNKNOWN_PRECISION, "Unsupported codec");
		}

		/* draw */
		draw_length =
		    (int)((float)bar_distance * (percent / (float)100.0));
		mvhline(2, 0, ' ', col);
		mvprintw(2, title_center + 2, "RMS:\t%d", (int)rms);
		mvhline(2 + x_padding, bar_start, ' ', bar_distance);
		mvhline(2 + x_padding, bar_start, '=', draw_length);
		move(1, 0);
		refresh();

		/* listen for input */
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
