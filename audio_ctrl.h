#ifndef AUDIO_CTRL_H
#define AUDIO_CTRL_H

#include <sys/audioio.h>

#define CTRL_MODE_PLAY "PLAY"
#define CTRL_MODE_RECORD "RECORD"

#define CTRL_CFG_PAUSE 1
#define CTRL_CFG_PLAY 0

#define ENC_MULAW "MU-LAW"
#define ENC_ALAW "A-LAW"
#define ENC_SLINEAR "SLINEAR"
#define ENC_SLINEAR_LE "SLINEAR_LE"
#define ENC_SLINEAR_BE "SLINEAR_BE"
#define ENC_ULINEAR "ULINEAR"
#define ENC_ULINEAR_LE "ULINEAR_LE"
#define ENC_ULINEAR_BE "ULINEAR_BE"
#define ENC_AC3 "DOLBY_DIGITAL_AC3"

#define ENC_OPTION_OFFSET 97

typedef struct audio_config_t {
	u_int buffer_size;
	u_int channels;
	u_int encoding;
	u_int precision;
	u_int sample_rate;
	u_int pause;
} audio_config_t;

typedef struct encoding_options_t {
	int total;
	audio_encoding_t *encodings;
} encoding_options_t;

typedef struct audio_ctrl_t {
	int fd;
	int mode;
	audio_config_t hw_info;
	audio_config_t config;
	encoding_options_t encoding_options;
	char *path;
} audio_ctrl_t;

int build_audio_ctrl(audio_ctrl_t *ctrl, char *path, int mode);
void print_ctrl(audio_ctrl_t ctrl);
void print_encodings(audio_ctrl_t *ctrl, int offset);
int get_max_encoding(audio_ctrl_t ctrl);
int update_ctrl(audio_ctrl_t *ctrl);
const char *get_encoding_name(int encoding);

#endif
