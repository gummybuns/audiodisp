#include <curses.h>
#include <math.h>
#include <sys/audioio.h>
#include <stdlib.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "audio_displays.h"


static float rms_mulaw(char *data, int start, int end) {
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
static float calculate_rms_percent(u_char *data, int encoding, int start, int end) {
	switch(encoding) {
		case AUDIO_ENCODING_ULAW:
			return rms_mulaw((char *) data, start, end) / 127 * 100;
		default:
			return -1;
	}
}

int check_options(int keypress)
{
	if (keypress == 'I') return DISPLAY_INFO;
	else if (keypress == 'R') return DISPLAY_RECORD;
	else if (keypress == 'E') return DISPLAY_ENCODING;
	else if (keypress == 'Q') return DISPLAY_EXIT;
	else return 0;
}

u_char display_info(audio_ctrl_t ctrl)
{
	int option;
	u_char keypress;

	move(0, 0);
	nodelay(stdscr, FALSE);
	print_ctrl(ctrl);
	while (1) {
		keypress = getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_INFO) return option;
	}
}

u_char display_encodings(audio_ctrl_t *ctrl)
{
	u_char keypress;
	int option;
	int index, max;
	audio_encoding_t encoding;

	mvprintw(0,0, "Available Encodings for %s\n", ctrl->path);
	printw("Enter a letter to update the encoding\n\n");
	nodelay(stdscr, FALSE);

	max = ctrl->encoding_options.total;

	while (1) {
		move(3, 0);
		print_encodings(ctrl);
		refresh();
		keypress = getch();
		option = check_options(keypress);

		if (option != 0 && option != DISPLAY_ENCODING) return option;

		if (
		    keypress >= ENC_OPTION_OFFSET
		    && keypress < max + ENC_OPTION_OFFSET
		) {
			printw("TRYING TO UPDATE ENCODING\n");
			index = keypress - ENC_OPTION_OFFSET;
			encoding = ctrl->encoding_options.encodings[index];
			int res = set_encoding(ctrl, encoding);
			printw("RESULT IS %d\n", res);
		}
	}
}

u_char display_intensity(audio_ctrl_t record_ctrl, audio_stream_t audio_stream)
{
	int row, col;
	int x_padding, y_padding;
	int bar_start, bar_end, bar_distance;
	int draw_length;
	int option;
	u_char keypress;

	getmaxyx(stdscr, row, col);
	y_padding = col / 10; 
	x_padding = row / 10;
	bar_start = y_padding;
	bar_end = col - y_padding;
	bar_distance = bar_end - bar_start;

	mvprintw(0, 0, "Measure Mic Intensity\n");
	mvprintw(3 + x_padding, bar_start, "0\%");
	mvprintw(3 + x_padding, bar_start + bar_distance/2, "50\%");
	mvprintw(3 + x_padding, bar_start + bar_distance - 3, "100\%");
	move(2*x_padding, 0);
	refresh();

	nodelay(stdscr, TRUE);
	while(1) {
		keypress = getch();
		option = check_options(keypress);
		if (option != 0 && option != DISPLAY_RECORD) return option;

		stream(record_ctrl, &audio_stream);
		// u_char is not gonna work right? cuz the data type is not always
		// gonna be a u_char?
		// TODO: u_char is not correct. that assumes it is using 8bit
		// precision which can change based on the selected encoding
		u_char *full_sample = flatten_stream(&audio_stream);

		int chunk_size = audio_stream.total_size;
		for (int i = 0; i < audio_stream.total_samples; i += chunk_size) {
			int z = (int) fmin(
			    (double) chunk_size,
			    (double) audio_stream.total_size - i
			);

			float percent = calculate_rms_percent(
			    full_sample,
			    audio_stream.encoding,
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
			}
		}
		free(full_sample);
	}
}

void display_options()
{
	int row, col;
	getmaxyx(stdscr, row, col);
	mvprintw(row-1, 0, "OPTIONS: ");
	printw("R: RECORD / I: INFO / E: ENCODINGS / Q: QUIT");
	refresh();
}
