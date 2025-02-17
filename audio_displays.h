#ifndef AUDIO_DISPLAYS_H
#define AUDIO_DISPLAYS_H

#include "audio_ctrl.h"
#include "audio_stream.h"

#define DISPLAY_EXIT -1
#define DISPLAY_RECORD 1
#define DISPLAY_INFO 2
#define DISPLAY_ENCODING 3

unsigned char display_info(audio_ctrl_t ctrl);
unsigned char display_intensity(audio_ctrl_t record_ctrl, audio_stream_t stream);
unsigned char  display_encodings(audio_ctrl_t *ctrl);
void display_options();
#endif
