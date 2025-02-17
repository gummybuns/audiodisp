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
#include "audio_displays.h"

#define STREAM_DURATION 250
#define ESC 27

#define mu 1E-11
#define DELAY_LINE
#define NUM_SECTIONS 128


// i think what i want this to do for now
// when u press R it records audio and does what it normally does
//   but in addition it stores the last 10 seconds of recorded data
// when u press P is plays the recorded audio back to u and shows the audio
//	 intensity again
// i just dont know how to get rid of the feedback loop
int main(int argc, char *argv[])
{
	u_char option;
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

	option = DISPLAY_RECORD;
	while (1) {
		// TODO: displaying the options at the bottom seems to add some delay
		// when switching windows.. im guessing because it has to scroll to
		// the very bottom?? seems kinda crazy
		display_options();

		if (option == DISPLAY_RECORD) {
			option = display_intensity(record_ctrl, stream1);
		} else if (option == DISPLAY_INFO) {
			option = display_info(record_ctrl);
		} else if (option == DISPLAY_ENCODING) {
			option = display_encodings(&record_ctrl);
		} else {
			break;
		}
		clear();
	}

	endwin();
	clean_buffers(&stream1);
	return 0;
}
