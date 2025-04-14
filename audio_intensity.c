#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <err.h>
#include <sys/audioio.h>
#include <math.h>

#include "audio_ctrl.h"
#include "audio_stream.h"
#include "audio_displays.h"

#define STREAM_DURATION 250

int main(int argc, char *argv[])
{
	char option;
	char *recording_audio_path;
	int result;
	audio_ctrl_t record_ctrl;
	audio_stream_t rstream;

	setprogname(argv[0]);

	if (argc <= 1) {
		err(1, "Specify a recording audio device");
	}

	recording_audio_path = argv[1];

	result = build_audio_ctrl(&record_ctrl, recording_audio_path, CTRL_RECORD);
	if (result != 0) {
		err(result, "Failed to build record audio controller %d", result);
	}

	initscr();
	raw();
	noecho();

	option = DISPLAY_RECORD;
	for (;;) {
		display_options();

		if (option == DISPLAY_RECORD) {
			result = build_stream_from_ctrl(
			    record_ctrl,
			    STREAM_DURATION,
			    &rstream);
			if (result != 0) {
				err(result, "Failed to build audio stream");
			}

			option = display_intensity(record_ctrl, &rstream);
			clean_buffers(&rstream);
		} else if (option == DISPLAY_INFO) {
			option = display_info(record_ctrl, rstream);
		} else {
			break;
		}
		clear();
	}

	endwin();
	return 0;
}
