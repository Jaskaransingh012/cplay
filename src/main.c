#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "library.h"
#include "wav.h"
#include "audio.h"
#include "decoder.h"
#include "terminal.h"
#include "player.h"

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


static int do_play(const char *path) {

    Player player;
    if (player_load(&player, path) != 0) {
        return CPLAY_EXIT_NOT_FOUND;
    }

    printf("Playing (%d Hz, %d ch, %d-bit)...\n",
           player.format.sample_rate, player.format.channels, player.format.bits_per_sample);

    printf("[p] Pause  [r] Resume  [q] Quit\n");

    if (terminal_enable_raw_mode() != 0) {
        fprintf(stderr, "Warning: could not enable raw terminal mode, controls disabled\n");
    }


    player_play(&player);

    int quit_requested = 0;

    while (player_tick(&player)) {

        if(terminal_was_interrupted()) {

            printf("\nInterrupted.\n");
            break;

        }

        char key = terminal_check_key();

        switch (key)
        {
        case 'p':
            /* code */
            player_pause(&player);
            printf("\rPaused.  ");
            fflush(stdout);
            break;

        case 'r':

            player_resume(&player);
            printf("\rResumed. ");
            fflush(stdout);
            break;

        case 'q':

            quit_requested = 1;
            break;

        default:
            break;
        }

        if(quit_requested) break;

    }

    if(quit_requested) {

        printf("\nStooped.\n");

    }
    else {

        printf("\nDone.\n");

    }

    player_stop(&player);
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
        return do_play(opts.query);
    }

    case CMD_UNKNOWN:
    default:
        fprintf(stderr, "Error: unknown command\n");
        fprintf(stderr, "Try `%s --help` for usage.\n", argv[0]);
        return CPLAY_EXIT_USAGE;
    }
}
