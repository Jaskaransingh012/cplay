#ifndef C_PLAY_MP3_H
#define C_PLAY_MP3_H
#include <stddef.h>

typedef struct Mp3File Mp3File;

int mp3_open(Mp3File **mp3, const char *path);
size_t mp3_read(Mp3File *mp3, void *buffer, size_t buffer_size);
void mp3_close(Mp3File *mp3);

#endif
