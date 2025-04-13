#include <sys/audioio.h>
#include <sys/ioctl.h>

#include <curses.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_ctrl.h"
#include "audio_stream.h"

static const char *
get_mode(audio_ctrl_t ctrl)
{
	if (ctrl.mode == AUMODE_PLAY)
		return CTRL_MODE_PLAY;
	else if (ctrl.mode == AUMODE_RECORD)
		return CTRL_MODE_RECORD;
	else
		return NULL;
}

/*
 * Print details about the audio controller
 */
void
print_ctrl(audio_ctrl_t ctrl)
{
	const char *mode, *config_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name((int)ctrl.config.encoding);

	printw(
	    "Audio Controller\n"
	    "\tDevice:\t\t%s\n"
	    "\tMode:\t\t%s\n"
	    "\tConfiguration:\n"
	    "\t\tbuffer_size:\t\t%d\n"
	    "\t\tsample_rate:\t\t%d\n"
	    "\t\tprecision:\t\t%d\n"
	    "\t\tchannels:\t\t%d\n"
	    "\t\tencoding:\t\t%s\n"
	    "\t\tpause:\t\t\t%d\n",
	    ctrl.path,
	    mode,
	    ctrl.config.buffer_size,
	    ctrl.config.sample_rate,
	    ctrl.config.precision,
	    ctrl.config.channels,
	    config_encoding,
	    ctrl.config.pause
	    );
}

/*
 * Initializes an audio controller based on the file path to the audio device
 * TODO figure out error codes
 */
int
build_audio_ctrl(audio_ctrl_t *ctrl, char *path, int mode)
{
	int fd;
	audio_info_t info, format;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return -1;
	}

	ctrl->path = path;
	ctrl->fd = fd;
	ctrl->mode = mode;

	/* initialize defaults */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return 1;
	}
	if (ioctl(ctrl->fd, AUDIO_GETFORMAT, &format) == -1) {
		return 1;
	}

	/* set device to use hardware's current settings */
	info.record.pause = format.record.pause;
	info.record.buffer_size = format.record.buffer_size;
	info.record.sample_rate = format.record.sample_rate;
	info.record.precision = format.record.precision;
	info.record.channels = format.record.channels;
	info.record.encoding = format.record.encoding;

	if (ioctl(ctrl->fd, AUDIO_SETINFO, &info) == -1) {
		return -3;
	}


	/* update ctrl to reflect changes */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return -1;
	}
	ctrl->config.precision = info.record.precision;
	ctrl->config.encoding = info.record.encoding;
	ctrl->config.buffer_size = info.record.buffer_size;
	ctrl->config.sample_rate = info.record.sample_rate;
	ctrl->config.channels = info.record.channels;

	return 0;
}

/*
 * Translate standard encoding definitions
 */
const char *
get_encoding_name(int encoding)
{
	switch (encoding) {
	case AUDIO_ENCODING_ULAW:
		return ENC_MULAW;
	case AUDIO_ENCODING_ALAW:
		return ENC_ALAW;
	case AUDIO_ENCODING_SLINEAR:
		return ENC_SLINEAR;
	case AUDIO_ENCODING_SLINEAR_LE:
		return ENC_SLINEAR_LE;
	case AUDIO_ENCODING_SLINEAR_BE:
		return ENC_SLINEAR_BE;
	case AUDIO_ENCODING_ULINEAR:
		return ENC_ULINEAR;
	case AUDIO_ENCODING_ULINEAR_LE:
		return ENC_ULINEAR_LE;
	case AUDIO_ENCODING_ULINEAR_BE:
		return ENC_ULINEAR_BE;
	case AUDIO_ENCODING_AC3:
		return ENC_AC3;
	default:
		return NULL;
	}
}
