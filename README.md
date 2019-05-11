# FP_SISOP_E04

### Dependencies
- libao v1.1.0
- mpg123 v1.13.8
- fuse

### How To Compile
``` bash
gcc -Wall `pkg-config fuse --cflags` mp3withfuse.c -o mp3withfuse `pkg-config fuse --libs` -lmpg123 -lao -pthread
```
