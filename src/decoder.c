#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"
#include "wav.h"
#include "mp3.h"

typedef enum {
    DECODER_TYPE_WAV,
    DECODER_TYPE_MP3
} DecoderType;


struct Decoder {
    DecoderType type;
    union {
        WavFile wav;
        Mp3File *mp3;
    } backend;
};

static int has_extension(const char *path, const char *ext) {

    size_t path_len = strlen(path);
    size_t ext_len  = strlen(ext);

    if(path_len < ext_len) return 0;

    const char *tail = path + (path_len - ext_len);

    for(size_t i = 0; i < ext_len; i++) {

        char a = tail[i];
        char b = ext[i];
        //convert it into lower case
        if(a >= 'A' && a <= 'Z') a += 32;
        if(b >= 'A' && b <= 'Z') b += 32;
        if(a != b) return 0;

    }
    return 1;
}

int decoder_open(Decoder **dec, const char *path)
{
    if(dec == NULL || path == NULL) return -1;

    *dec = NULL;

    Decoder *d = malloc(sizeof(Decoder));

    if (d == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        return -1;
    }

    if(has_extension(path, ".mp3")) {
        d->type = DECODER_TYPE_MP3;
        if(mp3_open(&d->backend.mp3, path) != 0) {
            free(d);
            return -1;
        }

    } else if (has_extension(path, ".wav")) {

        d->type = DECODER_TYPE_WAV;
        if(wav_open(&d->backend.wav, path) != 0) {
            free(d);
            return -1;
        }

    } else {

        fprintf(stderr, "Error unsupported file format\n");
        free(d);
        return -1;

    }

    *dec = d;

    return 0;
}


AudioFormat decoder_get_format(Decoder *dec)
{

    AudioFormat fmt;

    if(dec->type == DECODER_TYPE_WAV) {

        fmt.sample_rate      = dec->backend.wav.sample_rate;
        fmt.channels         = dec->backend.wav.channels;
        fmt.bits_per_sample  = dec->backend.wav.bits_per_sample;

    }
    else {
        fmt.sample_rate      = mp3_get_sample_rate(dec->backend.mp3);
        fmt.channels         = mp3_get_channels(dec->backend.mp3);
        fmt.bits_per_sample  = 16;
    }


    return fmt;
}

size_t decoder_read(Decoder *dec, void *buffer, size_t buffer_size)
{

    if(dec->type == DECODER_TYPE_WAV) {
        return wav_read(&dec->backend.wav, buffer, buffer_size);
    }else {
        return mp3_read(dec->backend.mp3, buffer, buffer_size);
    }

}

void decoder_close(Decoder *dec)
{
    if (dec == NULL) return;

    if(dec->type == DECODER_TYPE_WAV) {
        wav_close(&dec->backend.wav);
    } else {
        mp3_close(dec->backend.mp3);
    }

    free(dec);

}
