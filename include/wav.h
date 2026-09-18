#ifndef CPLAY_WAV_H
#define CPAY_WAV_H

#include <stdio.h>

typedef struct {
    FILE *file;
    int channels;
    int sample_rate;
    int bits_per_sample;
    unsigned int data_size;
    long data_offset;
    unsigned int bytes_remaining;
} WavFile;

int wav_open();
size_t wav_read();
void wav_close();



#endif
