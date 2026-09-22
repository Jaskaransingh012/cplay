#ifndef C_PLAY_PLAYER_H
#define C_PLAY_PLAYER_H

#include "decoder.h"
#include "audio.h"


typedef enum {
    PLAYER_STATE_LOADING,
    PLAYER_STATE_PAUSED,
    PLAYER_STATE_RESUME,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_STOPPED,
    PLAYER_STATE_FINISHED
} PlayerState;


typedef struct {

    Decoder *dec;
    AudioDevice *dev;
    PlayerState state;
    AudioFormat format;

} Player;


int player_load(Player *player, const char *path);
void player_play(Player *player);
void player_pause(Player *player);
void player_resume(Player *player);
void player_stop(Player *player);
const char *player_state_name(PlayerState state);


#endif
