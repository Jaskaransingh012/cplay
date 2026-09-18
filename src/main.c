#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "library.h"


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

    case CMD_UNKNOWN:
    default:
        fprintf(stderr, "Error: unknown command\n");
        fprintf(stderr, "Try `%s --help` for usage.\n", argv[0]);
        return CPLAY_EXIT_USAGE;
    }
}
