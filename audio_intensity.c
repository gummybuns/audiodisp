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
#define PLAY_DURATION 5000
#define STREAMS_NEEDED (PLAY_DURATION / STREAM_DURATION)

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
	char *play_audio_path;
	int result;
	audio_ctrl_t record_ctrl;
	audio_ctrl_t play_ctrl;
	audio_stream_t record_streams[STREAMS_NEEDED];
	circular_list_t stream_list;

	stream_list.size = STREAMS_NEEDED;
	stream_list.start = 0;
	stream_list.streams = record_streams;

	if (argc <= 2) {
		err(1, "Specify a recording audio device and playback devuce");
	}

	recording_audio_path = argv[1];
	play_audio_path = argv[2];

	result = build_audio_ctrl(&record_ctrl, recording_audio_path, AUMODE_RECORD);
	if (result != 0) {
		err(result, "Failed to build record audio controller %d", result);
	}

	result = build_audio_ctrl(&play_ctrl, play_audio_path, AUMODE_PLAY);
	if (result != 0) {
		err(result, "Failed to build play audio controller %d", result);
	}

	initscr();
	raw();
	noecho();

	option = DISPLAY_RECORD;
	while (1) {
		// TODO: displaying the options at the bottom seems to add some delay
		// when switching windows.. im guessing because it has to scroll to
		// the very bottom?? seems kinda crazy
		// TODO: memory leak - i need to clean up the buffers before i call
		// build_stream_from_ctrl
		display_options();

		if (option == DISPLAY_RECORD) {
			stream_list.start = 0;

			for (int i = 0; i < stream_list.size; i++) {
				result = build_stream_from_ctrl(
				    record_ctrl,
				    STREAM_DURATION,
				    &stream_list.streams[i]
				);
				if (result != 0) {
					err(result, "Failed to build audio stream");
				}
			}

			option = display_intensity(record_ctrl, &stream_list);
		} else if (option == DISPLAY_INFO) {
			stream_list.start = 0;

			// TODO - editing the encoding needs to reset streams instead
			for (int i = 0; i < stream_list.size; i++) {
				result = build_stream_from_ctrl(
				    record_ctrl,
				    STREAM_DURATION,
				    &stream_list.streams[i]
				);
				if (result != 0) {
					err(result, "Failed to build audio stream");
				}
			}
			option = display_info(record_ctrl, &stream_list);
		} else if (option == DISPLAY_ENCODING) {
			option = display_encodings(&record_ctrl, &play_ctrl);
		} else if (option == DISPLAY_PLAYBACK) {
			// TODO - if i change the encoding i should clear the list too
			option = display_playback(play_ctrl, &stream_list);
		} else {
			break;
		}
		clear();
	}

	endwin();

	// todo clean up the buffers in the stream_list
	return 0;
}
