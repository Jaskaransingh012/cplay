#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"

static unsigned int read_u32le(const unsigned char *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8) |
           ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}

static unsigned short read_u16le(const unsigned char *p)
{
    return (unsigned short)(p[0] | (p[1] << 8));
}

int wav_open(WavFile *wav, const char *path)
{
    memset(wav, 0, sizeof(*wav));

    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", path);
        return -1;
    }

    unsigned char header[44];
    if (fread(header, 1, 44, f) != 44) {
        fprintf(stderr, "Error: '%s' is too small to be a valid WAV file\n", path);
        fclose(f);
        return -1;
    }

    if (memcmp(header + 0, "RIFF", 4) != 0 ||
        memcmp(header + 8, "WAVE", 4) != 0 ||
        memcmp(header + 12, "fmt ", 4) != 0 ||
        memcmp(header + 36, "data", 4) != 0) {
        fprintf(stderr, "Error: '%s' is not a valid WAV file\n", path);
        fclose(f);
        return -1;
    }

    unsigned short audio_format = read_u16le(header + 20);
    if (audio_format != 1) {
        fprintf(stderr, "Error: '%s' uses an unsupported (non-PCM) WAV format\n", path);
        fclose(f);
        return -1;
    }

    wav->file = f;
    wav->channels = read_u16le(header + 22);
    wav->sample_rate = (int)read_u32le(header + 24);
    wav->bits_per_sample = read_u16le(header + 34);
    wav->data_size = read_u32le(header + 40);
    wav->data_offset = 44;
    wav->bytes_remaining = wav->data_size;

    if (wav->channels <= 0 || wav->sample_rate <= 0 || wav->bits_per_sample <= 0) {
        fprintf(stderr, "Error: '%s' has invalid audio format fields\n", path);
        fclose(f);
        return -1;
    }

    return 0;
}

size_t wav_read(WavFile *wav, void *buffer, size_t buffer_size)
{
    if (wav->bytes_remaining == 0) return 0;

    size_t to_read = buffer_size;
    if (to_read > wav->bytes_remaining) to_read = wav->bytes_remaining;

    size_t got = fread(buffer, 1, to_read, wav->file);
    wav->bytes_remaining -= (unsigned int)got;
    return got;
}

void wav_close(WavFile *wav)
{
    if (wav->file != NULL) {
        fclose(wav->file);
        wav->file = NULL;
    }
}
