#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/audioio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <curses.h>

#include "audio_ctrl.h"
#include "audio_stream.h"

const char *get_mode(audio_ctrl_t ctrl)
{
	if (ctrl.mode == AUMODE_PLAY) return CTRL_MODE_PLAY;
	else if(ctrl.mode == AUMODE_RECORD) return CTRL_MODE_RECORD;
	else return NULL;
}

void print_encodings(audio_ctrl_t *ctrl)
{
	audio_encoding_t enc;

	for (int i = 0; i < ctrl->encoding_options.total; i++) {
		enc = ctrl->encoding_options.encodings[i];
		printw(
		    "%c. %s (precision: %d)",
		    enc.index + ENC_OPTION_OFFSET,
		    get_encoding_name(enc.encoding),
			enc.precision
		);

		if (
		    enc.encoding == ctrl->config.encoding
		    && enc.precision == ctrl->config.precision
		) {
			printw("*");
		}

		printw("\n");
		enc.index++;
		
	}
}

void print_ctrl(audio_ctrl_t ctrl)
{
	const char *mode, *config_encoding, *hw_encoding;

	mode = get_mode(ctrl);
	config_encoding = get_encoding_name(ctrl.config.encoding);
	hw_encoding = get_encoding_name(ctrl.hw_info.encoding);

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
	  "\tHardware Information:\n"
	  "\t\tbuffer_size:\t\t%d\n"
	  "\t\tsample_rate:\t\t%d\n"
	  "\t\tprecision:\t\t%d\n"
	  "\t\tchannels:\t\t%d\n"
	  "\t\tencoding:\t\t%s\n",
	  ctrl.path,
      mode,
	  ctrl.config.buffer_size,
	  ctrl.config.sample_rate,
	  ctrl.config.precision,
	  ctrl.config.channels,
	  config_encoding,
	  ctrl.hw_info.buffer_size,
	  ctrl.hw_info.sample_rate,
	  ctrl.hw_info.precision,
	  ctrl.hw_info.channels,
	  hw_encoding
	);
}

static int set_config(int mode, audio_config_t *config, audio_info_t info)
{
	struct audio_prinfo prinfo;

	prinfo = mode == AUMODE_PLAY ? info.play : info.record;
	config->buffer_size = prinfo.buffer_size;
	config->sample_rate = prinfo.sample_rate;
	config->precision = prinfo.precision;
	config->channels = prinfo.channels;
	config->encoding = prinfo.encoding;

	return 0;
}

int build_audio_ctrl(audio_ctrl_t *ctrl, char *path, int mode)
{
	int open_flag, fd;
	audio_info_t info, format;
	audio_encoding_t enc;

	if (mode == AUMODE_PLAY) open_flag = O_WRONLY;
	else if (mode == AUMODE_RECORD) open_flag = O_RDONLY;
	else {
		printf("Invalid mode\n");
		return 3;
	}

	fd = open(path, open_flag);
	if (fd == -1) {
		printf("Failed to open %s\n", path);
		return -1;
	}

	ctrl->path = path;
	ctrl->fd = fd;
	ctrl->mode = mode;

	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) < 0) {
		printf("Failed to get info for %s\n", path);
		return 1;
	}
	if (ioctl(ctrl->fd, AUDIO_GETFORMAT, &format) < 0) {
		printf("Failed to get harware info for %s\n", path);
		return 1;
	}

	if (set_config(ctrl->mode, &(ctrl->config), info) != 0) {
		printf("Failed to set config\n");
		return 2;
	}

	if (set_config(ctrl->mode, &(ctrl->hw_info), format) != 0) {
		printf("Failed to set hw_info\n");
		return 2;
	}

	enc.index = 0;
	ctrl->encoding_options.total = 0;
	while (ioctl(ctrl->fd, AUDIO_GETENC, &enc) >= 0) {
		enc.index++;
		ctrl->encoding_options.total++;
	}

	ctrl->encoding_options.total -= 1;
	ctrl->encoding_options.encodings = malloc(
	    sizeof(audio_encoding_t) * ctrl->encoding_options.total
	);
	for (enc.index = 0; enc.index < ctrl->encoding_options.total; enc.index++) {
		encoding_options_t options = ctrl->encoding_options;
		audio_encoding_t *encoding = &options.encodings[enc.index];
		encoding->index = enc.index;
		ioctl(ctrl->fd, AUDIO_GETENC, encoding);
	}

	return 0;
}

int set_encoding(audio_ctrl_t *ctrl, audio_encoding_t encoding)
{
	audio_info_t info;
	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) < 0) {
		return -1;
	}

	if (ctrl->mode == AUMODE_PLAY) {
		info.play.encoding = encoding.encoding;
		info.play.precision = encoding.precision;
	} else if (ctrl->mode == AUMODE_RECORD) {
		info.record.encoding = encoding.encoding;
		info.record.precision = encoding.precision;
	} else {
		return -2;
	}

	if (ioctl(ctrl->fd, AUDIO_SETINFO, &info) < 0) {
		return -3;
	}

	if (ioctl(ctrl->fd, AUDIO_GETINFO, &info) < 0) {
		return -1;
	}

	if (set_config(ctrl->mode, &(ctrl->config), info) != 0) {
		printf("Failed to set config\n");
		return -4;
	}

	return 0;
}

const char *get_encoding_name(u_int encoding)
{
	if (encoding == AUDIO_ENCODING_ULAW) return ENC_MULAW;
	else if (encoding == AUDIO_ENCODING_ALAW) return ENC_ALAW;
	else if (encoding == AUDIO_ENCODING_SLINEAR) return ENC_SLINEAR;
	else if (encoding == AUDIO_ENCODING_SLINEAR_LE) return ENC_SLINEAR_LE;
	else if (encoding == AUDIO_ENCODING_SLINEAR_BE) return ENC_SLINEAR_BE;
	else if (encoding == AUDIO_ENCODING_ULINEAR) return ENC_ULINEAR;
	else if (encoding == AUDIO_ENCODING_ULINEAR_LE) return ENC_ULINEAR_LE;
	else if (encoding == AUDIO_ENCODING_ULINEAR_BE) return ENC_ULINEAR_BE;
	else if (encoding == AUDIO_ENCODING_AC3) return ENC_AC3;
	else return NULL;
}
