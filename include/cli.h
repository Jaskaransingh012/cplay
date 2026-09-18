#ifndef CPLAY_CLI_H
#define CPLAY_CLI_H


#define CPLAY_EXIT_OK          0
#define CPLAY_EXIT_ERROR       1
#define CPLAY_EXIT_USAGE       2
#define CPLAY_EXIT_NOT_FOUND   3
#define CPLAY_EXIT_IO_ERROR    4

// we have to make a struct of commands that user will enter
// that will be enum

typedef enum {
    CMD_NONE,  // NO ARGUMENT
    CMD_HELP, //ABOUT THE PLAYER
    CMD_SEARCH, // SEARCH THE MUSIC
    CMD_LIST, // list the songs in library
    CMD_UNKNOWN // NOT A VALID COMMAND
} CliCommand;

// for parsing commands and query
typedef struct {
    CliCommand command;
    const char* query;
} CliOptions;



// defining a parser
CliOptions cli_parser(int argc, char *argv[]);

void cli_help(const char *prog_name);


#endif
