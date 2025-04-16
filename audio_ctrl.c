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

/*
 * Translate standard encoding definitions
 */
const char *
get_encoding_name(ctrlencoding encoding)
{
	switch (encoding) {
	case CTRL_NONE:
		return "NONE";
	case CTRL_MULAW:
		return "MULAW";
	case CTRL_ALAW:
		return "ALAW";
	case CTRL_LINEAR:
		return "LINEAR";
	case CTRL_LINEAR8:
		return "LINEAR8";
	case CTRL_SLINEAR:
		return "SLINEAR";
	case CTRL_SLINEAR_LE:
		return "SLINEAR_LE";
	case CTRL_SLINEAR_BE:
		return "SLINEAR_BE";
	case CTRL_ULINEAR:
		return "ULINEAR";
	case CTRL_ULINEAR_LE:
		return "ULINEAR_LE";
	case CTRL_ULINEAR_BE:
		return "ULINEAR_BE";
	case CTRL_MPEG_L1_STREAM:
		return "MPEG_L1_STREAM";
	case CTRL_MPEG_L1_PACKETS:
		return "MPEG_L1_PACKETS";
	case CTRL_MPEG_L1_SYSTEM:
		return "MPEG_L1_SYSTEM";
	case CTRL_MPEG_L2_STREAM:
		return "MPEG_L2_STREAM";
	case CTRL_MPEG_L2_PACKETS:
		return "MPEG_L2_PACKETS";
	case CTRL_MPEG_L2_SYSTEM:
		return "MPEG_L2_SYSTEM";
	case CTRL_AC3:
		return "DOLBY_DIGITAL_AC3";
	default:
		return NULL;
	}
}

static const char *
get_mode(audio_ctrl_t ctrl)
{
	switch (ctrl.mode) {
	case CTRL_PLAY:
		return "PLAY";
	case CTRL_RECORD:
		return "RECORD";
	default:
		return NULL;
	}
}

/*
 * Print details about the audio controller
 */
void
print_ctrl(audio_ctrl_t ctrl)
{
	const char *mode, *config_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name(ctrl.config.encoding);

	printw("Audio Controller\n"
	       "\tDevice:\t\t%s\n"
	       "\tMode:\t\t%s\n"
	       "\tConfiguration:\n"
	       "\t\tbuffer_size:\t\t%d\n"
	       "\t\tsample_rate:\t\t%d\n"
	       "\t\tprecision:\t\t%d\n"
	       "\t\tchannels:\t\t%d\n"
	       "\t\tencoding:\t\t%s\n"
	       "\t\tpause:\t\t\t%d\n",
	    ctrl.path, mode, ctrl.config.buffer_size, ctrl.config.sample_rate,
	    ctrl.config.precision, ctrl.config.channels, config_encoding,
	    ctrl.config.pause);
}

/*
 * Initializes an audio controller based on the file path to the audio device
 */
int
build_audio_ctrl(audio_ctrl_t *ctrl, char *path, int mode)
{
	int fd;
	audio_info_t info, format;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return CTRL_ERR_FILE_OPEN;
	}

	ctrl->path = path;
	ctrl->fd = fd;
	ctrl->mode = mode;

	/* initialize defaults */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return CTRL_ERR_GETINFO;
	}
	if (ioctl(ctrl->fd, AUDIO_GETFORMAT, &format) == -1) {
		return CTRL_ERR_GETFORMAT;
	}

	/* set device to use hardware's current settings */
	info.record.pause = format.record.pause;
	info.record.buffer_size = format.record.buffer_size;
	info.record.sample_rate = format.record.sample_rate;
	info.record.precision = format.record.precision;
	info.record.channels = format.record.channels;
	info.record.encoding = format.record.encoding;

	if (ioctl(ctrl->fd, AUDIO_SETINFO, &info) == -1) {
		return CTRL_ERR_SETINFO;
	}

	/* update ctrl to reflect changes */
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) == -1) {
		return CTRL_ERR_GETINFO;
	}
	ctrl->config.precision = info.record.precision;
	ctrl->config.encoding = info.record.encoding;
	ctrl->config.buffer_size = info.record.buffer_size;
	ctrl->config.sample_rate = info.record.sample_rate;
	ctrl->config.channels = info.record.channels;

	return 0;
}
