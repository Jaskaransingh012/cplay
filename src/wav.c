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
    if (f == NULL)
    {
        fprintf(stderr, "Error: cannot open file '%s'\n", path);
        return -1;
    }

    /* read RIFF/WAVE header (first 12 bytes) */
    unsigned char riff[12];
    if (fread(riff, 1, 12, f) != 12 ||
        memcmp(riff + 0, "RIFF", 4) != 0 ||
        memcmp(riff + 8, "WAVE", 4) != 0)
    {
        fprintf(stderr, "Error: '%s' is not a valid WAV file\n", path);
        fclose(f);
        return -1;
    }

    int got_fmt = 0;
    int got_data = 0;

    /* walk chunks until we find fmt and data */
    while (!got_data)
    {
        unsigned char chunk_header[8];
        if (fread(chunk_header, 1, 8, f) != 8)
        {
            fprintf(stderr, "Error: '%s' ended before data chunk\n", path);
            fclose(f);
            return -1;
        }

        unsigned int chunk_size = read_u32le(chunk_header + 4);

        if (memcmp(chunk_header, "fmt ", 4) == 0)
        {
            if (chunk_size < 16)
            {
                fprintf(stderr, "Error: fmt chunk too small in '%s'\n", path);
                fclose(f);
                return -1;
            }
            unsigned char fmt[16];
            if (fread(fmt, 1, 16, f) != 16)
            {
                fprintf(stderr, "Error: cannot read fmt chunk in '%s'\n", path);
                fclose(f);
                return -1;
            }
            unsigned short audio_format = read_u16le(fmt + 0);

            if (audio_format == 65534)
            {
                if (chunk_size < 40)
                {
                    fprintf(stderr, "Error: extensible WAV fmt chunk too small in '%s'\n", path);
                    fclose(f);
                    return -1;
                }
                unsigned char ext[24];
                if (fread(ext, 1, 24, f) != 24)
                {
                    fprintf(stderr, "Error: cannot read extensible fmt in '%s'\n", path);
                    fclose(f);
                    return -1;
                }
                /* first 2 bytes of subformat GUID = actual codec */
                unsigned short subformat = read_u16le(ext + 8);
                if (subformat != 1)
                {
                    fprintf(stderr, "Error: '%s' extensible WAV is not PCM (subformat=%d)\n", path, subformat);
                    fclose(f);
                    return -1;
                }
                /* it is PCM — extract the remaining fields from already-read fmt bytes */
                wav->channels = read_u16le(fmt + 2);
                wav->sample_rate = (int)read_u32le(fmt + 4);
                wav->bits_per_sample = read_u16le(fmt + 14);

                /* skip remaining chunk bytes if any */
                long already_read = 16 + 24; /* base fmt + ext bytes */
                if ((long)chunk_size > already_read)
                {
                    fseek(f, (long)chunk_size - already_read, SEEK_CUR);
                }
                got_fmt = 1;
            }
            else if (audio_format != 1)
            {
                fprintf(stderr, "Error: '%s' is not PCM (format=%d)\n", path, audio_format);
                fclose(f);
                return -1;
            }
            else
            {
                /* plain PCM (format=1) — already read correctly above */
                wav->channels = read_u16le(fmt + 2);
                wav->sample_rate = (int)read_u32le(fmt + 4);
                wav->bits_per_sample = read_u16le(fmt + 14);
                if (chunk_size > 16)
                    fseek(f, (long)(chunk_size - 16), SEEK_CUR);
                got_fmt = 1;
            }
            wav->channels = read_u16le(fmt + 2);
            wav->sample_rate = (int)read_u32le(fmt + 4);
            wav->bits_per_sample = read_u16le(fmt + 14);

            /* skip any extra fmt bytes */
            if (chunk_size > 16)
            {
                fseek(f, (long)(chunk_size - 16), SEEK_CUR);
            }
            got_fmt = 1;
        }
        else if (memcmp(chunk_header, "data", 4) == 0)
        {
            if (!got_fmt)
            {
                fprintf(stderr, "Error: data chunk before fmt in '%s'\n", path);
                fclose(f);
                return -1;
            }
            wav->data_size = chunk_size;
            wav->bytes_remaining = chunk_size;
            wav->data_offset = ftell(f);
            got_data = 1;
        }
        else
        {
            /* unknown chunk — skip it */
            fseek(f, (long)chunk_size, SEEK_CUR);
        }
    }

    wav->file = f;
    return 0;
}

size_t wav_read(WavFile *wav, void *buffer, size_t buffer_size)
{
    if (wav->bytes_remaining == 0)
        return 0;

    size_t to_read = buffer_size;
    if (to_read > wav->bytes_remaining)
        to_read = wav->bytes_remaining;

    size_t got = fread(buffer, 1, to_read, wav->file);
    wav->bytes_remaining -= (unsigned int)got;
    return got;
    
}

void wav_close(WavFile *wav)
{
    if (wav->file != NULL)
    {
        fclose(wav->file);
        wav->file = NULL;
    }
}
