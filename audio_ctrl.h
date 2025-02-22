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

#define ENC_OPTION_OFFSET 97

typedef struct audio_config_t {
	unsigned int buffer_size;
	unsigned int channels;
	unsigned int encoding;
	unsigned int precision;
	unsigned int sample_rate;
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

int build_audio_ctrl(audio_ctrl_t * ctrl, char *path, int mode);
void print_ctrl(audio_ctrl_t ctrl);
void print_encodings(audio_ctrl_t * ctrl, int offset);
int get_max_encoding(audio_ctrl_t ctrl);
int set_encoding(audio_ctrl_t * ctrl, audio_encoding_t encoding);
const char *get_encoding_name(u_int encoding);

#endif
