#ifndef CPLAY_AUDIO_H
#define CPLAY_AUDIO_H

#include <stddef.h>

typedef struct AudioDevice AudioDevice;

int audio_open(AudioDevice **dev, int sample_rate, int channels, int bits_per_sample);
int audio_write(AudioDevice *dev, const void *buffer, size_t frames);
void audio_close(AudioDevice *dev);

#endif
