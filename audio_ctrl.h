#ifndef AUDIO_CTRL_H
#define AUDIO_CTRL_H

#include <sys/audioio.h>

#define CTRL_CFG_PAUSE 1
#define CTRL_CFG_PLAY 0

typedef enum ctrlmode {
	CTRL_PLAY = AUMODE_PLAY,
	CTRL_RECORD = AUMODE_RECORD
} ctrlmode;

typedef enum ctrlencoding {
	CTRL_NONE = AUDIO_ENCODING_NONE,
	CTRL_MULAW = AUDIO_ENCODING_ULAW,
	CTRL_ALAW = AUDIO_ENCODING_ALAW,
	CTRL_ADPCM = AUDIO_ENCODING_ADPCM,
	CTRL_LINEAR = AUDIO_ENCODING_LINEAR,
	CTRL_LINEAR8 = AUDIO_ENCODING_LINEAR8,
	CTRL_SLINEAR = AUDIO_ENCODING_SLINEAR,
	CTRL_SLINEAR_LE = AUDIO_ENCODING_SLINEAR_LE,
	CTRL_SLINEAR_BE = AUDIO_ENCODING_SLINEAR_BE,
	CTRL_ULINEAR = AUDIO_ENCODING_ULINEAR,
	CTRL_ULINEAR_LE = AUDIO_ENCODING_ULINEAR_LE,
	CTRL_ULINEAR_BE = AUDIO_ENCODING_ULINEAR_BE,
	CTRL_MPEG_L1_STREAM = AUDIO_ENCODING_MPEG_L1_STREAM,
	CTRL_MPEG_L1_PACKETS = AUDIO_ENCODING_MPEG_L1_PACKETS,
	CTRL_MPEG_L1_SYSTEM = AUDIO_ENCODING_MPEG_L1_SYSTEM,
	CTRL_MPEG_L2_STREAM = AUDIO_ENCODING_MPEG_L2_STREAM,
	CTRL_MPEG_L2_PACKETS = AUDIO_ENCODING_MPEG_L2_PACKETS,
	CTRL_MPEG_L2_SYSTEM = AUDIO_ENCODING_MPEG_L2_SYSTEM,
	CTRL_AC3 = AUDIO_ENCODING_AC3
} ctrlencoding;

typedef struct audio_config_t {
	u_int buffer_size;     /* size of the audio device buffer in bytes */
	u_int channels;        /* number of channels for the audio device */
	ctrlencoding encoding; /* the encoding of the audio device */
	u_int precision;       /* the number of bits per sample */
	u_int sample_rate;     /* number of samples per second */
	u_int pause;           /* is the device actively recording */
} audio_config_t;

typedef struct audio_ctrl_t {
	int fd;                /* file descriptor to the audio device */
	ctrlmode mode;         /* record vs play */
	audio_config_t config; /* the configuration of the audio device */
	char *path;            /* the path to the audio device */
} audio_ctrl_t;

int build_audio_ctrl(audio_ctrl_t *ctrl, char *path, int mode);
void print_ctrl(audio_ctrl_t ctrl);
const char *get_encoding_name(ctrlencoding encoding);

#endif
