#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "library.h"
#include "wav.h"
#include "audio.h"
#include "decoder.h"

static char *get_music_dir(void)
{
    const char *home = getenv("HOME"); // read environment variables from the os
    if (home == NULL) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        return NULL;
    }

    size_t len = strlen(home) + strlen("/Music") + 1;
    char *path = malloc(len);
    if (path == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        return NULL;
    }
    snprintf(path, len, "%s/Music", home);
    return path;
}


static int do_play_wav(const char *path) {

    Decoder *dec;

    if(decoder_open(&dec, path) != 0) {
        return CPLAY_EXIT_NOT_FOUND;
    }

    // Audio format extraction from decoder
    AudioFormat fmt = decoder_get_format(dec);

    // Audio device config
    AudioDevice *dev = NULL;

    if(audio_open(&dev, fmt.sample_rate, fmt.channels, fmt.bits_per_sample) != 0) {
        return CPLAY_EXIT_ERROR;
    }

    unsigned char buffer[4096];
    size_t bytes_read;
    size_t bytes_per_frame = (size_t)fmt.channels * (size_t)(fmt.bits_per_sample / 8);

    printf("Playing (%d Hz, %d ch, %d-bit)...\n", fmt.sample_rate, fmt.channels, fmt.bits_per_sample);

    while((bytes_read = decoder_read(dec, buffer, sizeof(buffer))) > 0) {

        size_t frames = bytes_read / bytes_per_frame;
        if(audio_write(dev, buffer, frames) < 0) break;

    }

    printf("Done\n");

    audio_close(dev);
    decoder_close(dec);

    return CPLAY_EXIT_OK;

}



int main(int argc, char *argv[])
{
    CliOptions opts = cli_parser(argc, argv);

    switch (opts.command) {

    case CMD_HELP:
        cli_help(argv[0]);
        return CPLAY_EXIT_OK;

    case CMD_LIST: {
        char *music_dir = get_music_dir();
        if (music_dir == NULL) {
            return CPLAY_EXIT_ERROR;
        }

        Library lib;

        library_init(&lib); // initialize the library with 0 songs and 0 capacity

        if (library_scan(&lib, music_dir) != 0) {

            free(music_dir);
            library_free(&lib);
            return CPLAY_EXIT_IO_ERROR;

        }

        library_print(&lib);

        library_free(&lib);
        free(music_dir);
        return CPLAY_EXIT_OK;
    }

    case CMD_SEARCH: {
        if (opts.query == NULL) {
            fprintf(stderr, "Error: search requires a query, e.g. cplay search \"Believer\"\n");
            return CPLAY_EXIT_USAGE;
        }

        char *music_dir = get_music_dir();
        if (music_dir == NULL) {
            return CPLAY_EXIT_ERROR;
        }

        Library lib;
        library_init(&lib);
        if (library_scan(&lib, music_dir) != 0) {
            fprintf(stderr, "Hint: create it with: mkdir -p %s\n", music_dir);
            free(music_dir);
            library_free(&lib);
            return CPLAY_EXIT_IO_ERROR;
        }

        size_t match_count = 0;
        Song **matches = library_search(&lib, opts.query, &match_count);

        if (match_count == 0) {
            printf("Error: song not found: \"%s\"\n", opts.query);
            printf("Searched in: %s\n", music_dir);
            free(matches);
            library_free(&lib);
            free(music_dir);
            return CPLAY_EXIT_NOT_FOUND;
        }

        printf("Found %zu match(es):\n", match_count);
        for (size_t i = 0; i < match_count; i++) {
            printf("  %zu. %s - %s\n", i + 1, matches[i]->artist, matches[i]->title);
        }

        free(matches);
        library_free(&lib);
        free(music_dir);
        return CPLAY_EXIT_OK;
    }

    case CMD_NONE:
        printf("cplay: no arguments given. Try `cplay --help`.\n");
        return CPLAY_EXIT_OK;

    case CMD_PLAY: {
        if (opts.query == NULL) {
            fprintf(stderr, "Error: play requires a file path, e.g. cplay play song.wav\n");
            return CPLAY_EXIT_USAGE;
        }
        return do_play_wav(opts.query);
    }

    case CMD_UNKNOWN:
    default:
        fprintf(stderr, "Error: unknown command\n");
        fprintf(stderr, "Try `%s --help` for usage.\n", argv[0]);
        return CPLAY_EXIT_USAGE;
    }
}
