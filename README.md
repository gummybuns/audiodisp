## Build

```bash
gcc audio_intensity.c audio_ctrl.c audio_stream.c -lm -lcurses -o bin/audiodisp

bin/audiodisp /dev/audio2
```
