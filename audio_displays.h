#ifndef AUDIO_DISPLAYS_H
#define AUDIO_DISPLAYS_H

#include "audio_ctrl.h"
#include "audio_stream.h"

#define DISPLAY_EXIT -1
#define DISPLAY_RECORD 1
#define DISPLAY_INFO 2

char display_info(audio_ctrl_t ctrl, audio_stream_t audio_stream);
char display_intensity(audio_ctrl_t rctrl, audio_stream_t *audio_stream);
void display_options(void);
#endif
