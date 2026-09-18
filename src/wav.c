#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wav.h"


int wav_open(WavFile *wav, const char *path)
{
    memset(wav, 0, sizeof(*wav));

    FILE *f = fopen(path, "rb");

    if (f == NULL) {
        fprintf(stderr,
                "Error: cannot open file '%s'\n",
                path);
        return -1;
    }


    WaveHeader header;

    if (fread(&header, 1, sizeof(WaveHeader), f) != sizeof(WaveHeader)) {

        fprintf(stderr,
                "Error: '%s' is too small to be a valid WAV file\n",
                path);

        fclose(f);
        return -1;
    }


    if (memcmp(header.riff, "RIFF", 4) != 0 ||
        memcmp(header.wave, "WAVE", 4) != 0 ||
        memcmp(header.fmt,  "fmt ", 4) != 0 ||
        memcmp(header.data, "data", 4) != 0)
    {
        fprintf(stderr,
                "Error: '%s' is not a valid WAV file\n",
                path);

        fclose(f);
        return -1;
    }


    if (header.audio_format != 1) {

        fprintf(stderr,
                "Error: '%s' uses an unsupported "
                "(non-PCM) WAV format\n",
                path);

        fclose(f);
        return -1;
    }

    if (header.channels == 0 ||
        header.sample_rate == 0 ||
        header.bits_per_sample == 0)
    {
        fprintf(stderr,
                "Error: '%s' has invalid audio format fields\n",
                path);

        fclose(f);
        return -1;
    }


  
    wav->file = f;

    wav->channels =
        header.channels;

    wav->sample_rate =
        (int)header.sample_rate;

    wav->bits_per_sample =
        header.bits_per_sample;

    wav->data_size =
        header.data_size;

    wav->data_offset =
        sizeof(WaveHeader);

    wav->bytes_remaining =
        header.data_size;


    return 0;
}


size_t wav_read(WavFile *wav,
                void *buffer,
                size_t buffer_size)
{
    if (wav->bytes_remaining == 0)
        return 0;


    size_t to_read = buffer_size;

    if (to_read > wav->bytes_remaining)
        to_read = wav->bytes_remaining;


    size_t got =
        fread(buffer, 1, to_read, wav->file);


    wav->bytes_remaining -=
        (unsigned int)got;


    return got;
}


void wav_close(WavFile *wav)
{
    if (wav->file != NULL) {

        fclose(wav->file);

        wav->file = NULL;
    }
}
