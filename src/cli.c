#include <stdio.h>
#include <string.h>
#include "cli.h"

CliOptions cli_parser(int argc, char *argv[]) {
    CliOptions options;
    options.command = CMD_NONE;
    options.query = NULL;

    if(argc < 2) {
        // No argument are given properly
        return options;
    }

    const char *first = argv[1];

    if(strcmp(first, "--help") == 0 || strcmp(first, "-h")==0 ) {
        options.command = CMD_HELP;
        return options;
    }

    if (strcmp(first, "--list") == 0) {
        options.command = CMD_LIST;
        return options;
    }

    if(strcmp(first, "search") == 0) {
        options.command = CMD_SEARCH;
        if(argc >= 3) {
            options.query = argv[2];
        }
        return options;
    }

    options.command = CMD_UNKNOWN;
    return options;

}

void cli_help(const char *prog_name)
{
    printf("cplay - a command-line music player\n\n");
    printf("Usage:\n");
    printf("  %s \"song name\"       Search and play a song\n", prog_name);
    printf("  %s search \"song\"     Search the local library\n", prog_name);
    printf("  %s --list             List all songs in ~/Music\n", prog_name);
    printf("  %s --help             Show this help message\n", prog_name);
    printf("  %s --version          Show version information\n", prog_name);
    printf("\n");
    printf("More commands (play, pause, resume, queue, ...) are coming in later phases.\n");
}

