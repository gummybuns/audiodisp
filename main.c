#include <curses.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/audioio.h>
#include <unistd.h>

#include "audio_ctrl.h"
#include "audio_displays.h"
#include "audio_stream.h"
#include "error_codes.h"

#define STREAM_DURATION 250

int
main(int argc, char *argv[])
{
	int option;
	int res;
	char *path;
	audio_ctrl_t rctrl;
	audio_stream_t rstream;

	setprogname(argv[0]);

	if (argc <= 1) {
		err(1, "Specify a recording audio device");
	}

	path = argv[1];

	res = build_audio_ctrl(&rctrl, path, CTRL_RECORD);
	if (res != 0) {
		err(1, "Failed to build record audio controller: %d", res);
	}

	initscr();
	raw();
	noecho();

	option = DISPLAY_RECORD;
	for (;;) {
		display_options();

		if (option >= E_UNHANDLED) {
			err(1, "Unhandled Error: %d", option);
		}

		if (option == DISPLAY_RECORD) {
			res = build_stream_from_ctrl(rctrl, STREAM_DURATION,
			    &rstream);
			if (res != 0) {
				err(1, "Failed to build audio stream: %d", res);
			}

			option = display_intensity(rctrl, &rstream);
			clean_buffers(&rstream);
		} else if (option == DISPLAY_INFO) {
			option = display_info(rctrl, rstream);
		} else {
			break;
		}
		clear();
	}

	endwin();
	return 0;
}
