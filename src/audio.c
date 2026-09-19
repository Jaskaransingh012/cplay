#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "audio.h"

struct AudioDevice {
    snd_pcm_t *handle;
};

int audio_open(AudioDevice **dev, int sample_rate, int channels, int bits_per_sample)
{
    AudioDevice *d = malloc(sizeof(AudioDevice));
    if (d == NULL) {
        fprintf(stderr, "Error: out of memory opening audio device\n");
        return -1;
    }

    snd_pcm_format_t format;
    switch (bits_per_sample) {
        case 16: format = SND_PCM_FORMAT_S16_LE; break;
        case 8:  format = SND_PCM_FORMAT_U8;      break;
        default:
            fprintf(stderr, "Error: unsupported bit depth: %d\n", bits_per_sample);
            free(d);
            return -1;
    }

    int err = snd_pcm_open(&d->handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "Error: cannot open audio device: %s\n", snd_strerror(err));
        free(d);
        return -1;
    }

    err = snd_pcm_set_params(d->handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                              (unsigned int)channels, (unsigned int)sample_rate,
                              1, 500000);
    if (err < 0) {
        fprintf(stderr, "Error: cannot configure audio device: %s\n", snd_strerror(err));
        snd_pcm_close(d->handle);
        free(d);
        return -1;
    }

    *dev = d;
    return 0;
}

int audio_write(AudioDevice *dev, const void *buffer, size_t frames)
{
    snd_pcm_sframes_t written = snd_pcm_writei(dev->handle, buffer, frames);

    if (written == -EPIPE) {
        /* buffer underrun: recover and retry once */
        snd_pcm_prepare(dev->handle);
        written = snd_pcm_writei(dev->handle, buffer, frames);
    }

    if (written < 0) {
        fprintf(stderr, "Error: audio write failed: %s\n", snd_strerror((int)written));
        return -1;
    }

    return (int)written;
}

void audio_close(AudioDevice *dev)
{
    if (dev == NULL) return;
    snd_pcm_drain(dev->handle);
    snd_pcm_close(dev->handle);
    free(dev);
}
