#ifndef AUDIO_CTRL_H
#define AUDIO_CTRL_H

#define CTRL_MODE_PLAY "PLAY"
#define CTRL_MODE_RECORD "RECORD"

#define ENC_MULAW "MU-LAW"
#define ENC_ALAW "A-LAW"
#define ENC_SLINEAR "SLINEAR"
#define ENC_SLINEAR_LE "SLINEAR_LE"
#define ENC_SLINEAR_BE "SLINEAR_BE"
#define ENC_ULINEAR "ULINEAR"
#define ENC_ULINEAR_LE "ULINEAR_LE"
#define ENC_ULINEAR_BE "ULINEAR_BE"
#define ENC_AC3 "DOLBY_DIGITAL_AC3"

typedef struct audioconfig {
	u_int sample_rate;
	u_int precision;
	u_int channels;
	u_int encoding;
    u_int buffer_size;
} audio_config_t;

typedef struct audio_controller {
	char *path;
	int fd;
	int mode;
	audio_config_t config;
	audio_config_t hw_info;
} audio_ctrl_t;


int build_audio_ctrl(audio_ctrl_t *ctrl, char *path, int mode);
void print_ctrl(audio_ctrl_t ctrl);
const char *get_encoding(u_int encoding);
const char *get_mode(audio_ctrl_t ctrl);

#endif
