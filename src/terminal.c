#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include "terminal.h"

static struct termios orig_termios;
static volatile sig_atomic_t g_interrupted = 0;

static void handle_sigint(int sig)
{
    (void)sig;
    g_interrupted = 1;
}

void terminal_disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int terminal_enable_raw_mode(void)
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        return -1;
    }

    atexit(terminal_disable_raw_mode);
    signal(SIGINT, handle_sigint);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return -1;
    }

    return 0;
}

char terminal_check_key(void)
{
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        return c;
    }
    return 0;
}

int terminal_was_interrupted(void)
{
    return g_interrupted;
}
