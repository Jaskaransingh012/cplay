#ifndef CPLAY_WAV_H
#define CPAY_WAV_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


typedef struct {
    FILE *file;
    int channels;
    int sample_rate;
    int bits_per_sample;
    unsigned int data_size;
    long data_offset;
    unsigned int bytes_remaining;
} WavFile;


typedef struct {
    char     riff[4];
    uint32_t file_size;
    char     wave[4];

    char     fmt[4];
    uint32_t fmt_size;

    uint16_t audio_format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;

    char     data[4];
    uint32_t data_size;

} WaveHeader;

int wav_open(WavFile *wav, const char *path);
size_t wav_read(WavFile *wav, void *buffer, size_t buffer_size);
void wav_close(WavFile *wav);



#endif
