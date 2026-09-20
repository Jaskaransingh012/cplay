#ifndef C_PLAY_DECODER_H
#define C_PLAY_DECODER_H

#include <stddef.h>

typedef struct Decoder Decoder;

typedef struct {

    int sample_rate; // -> how many audio measurments are taken every second eg -> 4400 htz

    int channels; // -> how many seperate audio streams are stored

    int bits_per_sample; // -> how many bits are used to represent the one sample

} AudioFormat;

int decoder_open(Decoder **dec, const char *path);

AudioFormat decoder_get_format(Decoder *dec);

size_t decoder_read(Decoder *dec, void *buffer, size_t buffer_size);

void decoder_close;

#endif
