## Build

```bash
pkgin install ncurses
pkgin instal kissfft

gcc -I/usr/pkg/include audio_intensity.c audio_ctrl.c audio_stream.c -lm -lcurses -o bin/audiodisp
```
