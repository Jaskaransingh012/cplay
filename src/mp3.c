#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpg123.h>
#include "mp3.h"

struct Mp3File {

    mpg123_handle *handle;

    int sample_rate;
    int channels;
    int encoding;

    long total_samples;

};

int mp3_open(Mp3File **mp3, const char *path) {

    if(mp3 == NULL || path == NULL) {
        return -1;
    }

    *mp3 = NULL;

    int err;

    // Initialize the library of mpg123
    if(mpg123_init() != MPG123_OK) {

        fprintf(stderr, "Error, failed to initialize the mpg123 library\n");
        return -1;

    }

    // decoder handler
    mpg123_handle *handle = mpg123_new(NULL , &err);

    if(handle == NULL) {

        fprintf(stderr, "Error, cannot create a hadler of mpg123\n");
        mpg123_delete(handle);
        mpg123_exit();

        return -1;

    }

    long sample_rate;
    int channels;
    int encoding;

    if(mpg123_getformat(handle, &sample_rate, &channels, &encoding) != MPG123_OK) {

        fprintf(stderr, "Error : cannot get the mp3 format\n");
        mpg123_close(handle);
        mpg123_delete(handle);
        mpg123_exit();

        return -1;

    }

    Mp3File *file = malloc(sizeof(Mp3File));

    if(file == NULL) {

        fprintf(stderr, "Error : cannot open the file\n");
        mpg123_close(handle);
        mpg123_delete(handle);
        mpg123_exit();

        return -1;

    }

    file->handle = handle;
    file->sample_rate = sample_rate;
    file->channels = channels;
    file->encoding = encoding;
    file->total_samples = mpg123_length(handle);

    *mp3 = file;

    return 0;

}

size_t mp3_read(Mp3File *mp3, void *buffer, size_t buffer_size) {

    if (mp3 == NULL || mp3->handle == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }

    size_t bytes_read = 0;

    int result = mpg123_read(
        mp3->handle,
        buffer,
        buffer_size,
        &bytes_read
    );

     if (result != MPG123_OK && result != MPG123_DONE &&  result != MPG123_NEW_FORMAT) {

        fprintf(stderr,
                "Error while decoding MP3: %s\n",
                mpg123_strerror(mp3->handle));

        return 0;

    }

    return bytes_read;

}

void mp3_close(Mp3File *mp3) {

    if(mp3 == NULL) return;

    if(mp3->handle != NULL) {
        mpg123_close(mp3->handle);
        mpg123_delete(mp3->handle);
    }

    free(mp3);

    mpg123_exit();

}
