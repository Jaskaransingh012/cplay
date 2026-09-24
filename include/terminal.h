#ifndef CPLAY_TERMINAL_H
#define CPLAY_TERMINAL_H



int terminal_enable_raw_mode(void);

void terminal_disable_raw_mode(void);

char terminal_check_key(void);

int terminal_was_interrupted(void);

#endif
