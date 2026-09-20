#include <stdio.h>
#include <stdlib.h>
#include "decoder.h"
#include "wav.h"

struct Decoder {
    WavFile wav;
};

int decoder_open(Decoder **dec, const char *path)
{
    Decoder *d = malloc(sizeof(Decoder));
    if (d == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        return -1;
    }

    if (wav_open(&d->wav, path) != 0) {
        free(d);
        return -1;
    }

    *dec = d;
    return 0;
}

AudioFormat decoder_get_format(Decoder *dec)
{
    AudioFormat fmt;
    fmt.sample_rate      = dec->wav.sample_rate;
    fmt.channels         = dec->wav.channels;
    fmt.bits_per_sample  = dec->wav.bits_per_sample;
    return fmt;
}

size_t decoder_read(Decoder *dec, void *buffer, size_t buffer_size)
{
    return wav_read(&dec->wav, buffer, buffer_size);
}

void decoder_close(Decoder *dec)
{
    if (dec == NULL) return;
    wav_close(&dec->wav);
    free(dec);
}
