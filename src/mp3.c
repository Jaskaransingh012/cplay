#include <stdio.h>
#include <stdlib.h>
#include <mpg123.h>
#include "mp3.h"

struct Mp3File {
    mpg123_handle *handle;
    int sample_rate;
    int channels;
};

int mp3_global_init(void)
{
    if (mpg123_init() != MPG123_OK) {
        fprintf(stderr, "Error: failed to initialize mpg123 library\n");
        return -1;
    }
    return 0;
}

void mp3_global_exit(void)
{
    mpg123_exit();
}

int mp3_open(Mp3File **mp3, const char *path)
{
    if (mp3 == NULL || path == NULL) return -1;
    *mp3 = NULL;

    int err;
    mpg123_handle *handle = mpg123_new(NULL, &err);
    if (handle == NULL) {
        fprintf(stderr, "Error: cannot create mpg123 handle\n");
        return -1;
    }

    /* Allow every sample rate mpg123 knows, but ONLY 16-bit signed output.
     * This must be set BEFORE mpg123_open. */
    mpg123_format_none(handle);
    const long *rates;
    size_t rate_count;
    mpg123_rates(&rates, &rate_count);
    for (size_t i = 0; i < rate_count; i++) {
        mpg123_format(handle, rates[i], MPG123_STEREO | MPG123_MONO, MPG123_ENC_SIGNED_16);
    }

    if (mpg123_open(handle, path) != MPG123_OK) {
        fprintf(stderr, "Error: cannot open mp3 file '%s': %s\n", path, mpg123_strerror(handle));
        mpg123_delete(handle);
        return -1;
    }

    long sample_rate;
    int channels, encoding;
    if (mpg123_getformat(handle, &sample_rate, &channels, &encoding) != MPG123_OK) {
        fprintf(stderr, "Error: cannot get mp3 format for '%s'\n", path);
        mpg123_close(handle);
        mpg123_delete(handle);
        return -1;
    }

    Mp3File *file = malloc(sizeof(Mp3File));
    if (file == NULL) {
        fprintf(stderr, "Error: out of memory opening '%s'\n", path);
        mpg123_close(handle);
        mpg123_delete(handle);
        return -1;
    }

    file->handle = handle;
    file->sample_rate = (int)sample_rate;
    file->channels = channels;

    *mp3 = file;
    return 0;
}

size_t mp3_read(Mp3File *mp3, void *buffer, size_t buffer_size)
{
    if (mp3 == NULL || mp3->handle == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }

    size_t bytes_read = 0;
    int result = mpg123_read(mp3->handle, buffer, buffer_size, &bytes_read);

    if (result != MPG123_OK && result != MPG123_DONE && result != MPG123_NEW_FORMAT) {
        fprintf(stderr, "Error while decoding MP3: %s\n", mpg123_strerror(mp3->handle));
        return 0;
    }

    return bytes_read;
}

void mp3_close(Mp3File *mp3)
{
    if (mp3 == NULL) return;

    if (mp3->handle != NULL) {
        mpg123_close(mp3->handle);
        mpg123_delete(mp3->handle);
    }
    free(mp3);
}

int mp3_get_sample_rate(Mp3File *mp3) { return mp3->sample_rate; }
int mp3_get_channels(Mp3File *mp3)    { return mp3->channels; }
