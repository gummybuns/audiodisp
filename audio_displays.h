#ifndef AUDIO_DISPLAYS_H
#define AUDIO_DISPLAYS_H

#include "audio_ctrl.h"
#include "audio_stream.h"

#define DISPLAY_EXIT -1
#define DISPLAY_RECORD 1
#define DISPLAY_INFO 2
#define DISPLAY_ENCODING 3
#define DISPLAY_PLAYBACK 4

char display_info(audio_ctrl_t ctrl, circular_list_t * slist);
char display_intensity(audio_ctrl_t rctrl, circular_list_t * slist);
char display_encodings(audio_ctrl_t * rctrl, audio_ctrl_t * pctrl);
char display_playback(audio_ctrl_t ctrl, circular_list_t * slist);
void display_options(void);
#endif
