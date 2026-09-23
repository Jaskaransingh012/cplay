#include <stdio.h>

#include "cli.h"
#include "decoder.h"
#include "player.h"
#include "audio.h"


int player_load(Player *player, const char *path) {

    player->state = PLAYER_STATE_LOADING;
    Decoder *dec;

    if(decoder_open(&dec, path) != 0) {
        return CPLAY_EXIT_NOT_FOUND;
    }

    AudioFormat fmt = decoder_get_format(dec);

    // Audio device config
    AudioDevice *dev = NULL;

    if(audio_open(&dev, fmt.sample_rate, fmt.channels, fmt.bits_per_sample) != 0) {
        return CPLAY_EXIT_ERROR;
    }

    player->dec = dec;
    player->dev = dev;
    player->format = fmt;

    return 0;

}

int  player_tick(Player *player){

    if(player->state != PLAYER_STATE_PLAYING) {
        return 1;
    }

    unsigned char buffer[4096];

    size_t bytes_read = decoder_read(player->dec, buffer, sizeof(buffer));

    if(bytes_read <= 0) {
        player->state = PLAYER_STATE_FINISHED;
        return 0;
    }

    size_t bytes_per_frame = (size_t)player->format.channels * (size_t)(player->format.bits_per_sample / 8);

    size_t frames = bytes_read / bytes_per_frame;

    audio_write(player->dev, buffer, frames);

    return 1;

}

void player_play(Player *player) {

    if(player->state == PLAYER_STATE_FINISHED) return;
    player->state = PLAYER_STATE_PLAYING;

}

void player_pause(Player *player)
{
    if (player->state == PLAYER_STATE_PLAYING) {
        player->state = PLAYER_STATE_PAUSED;
    }
}

void player_resume(Player *player)
{
    if (player->state == PLAYER_STATE_PAUSED) {
        player->state = PLAYER_STATE_PLAYING;
    }
}

void player_stop(Player *player)
{
    if (player->dev != NULL) {
        audio_close(player->dev);
        player->dev = NULL;
    }
    if (player->dec != NULL) {
        decoder_close(player->dec);
        player->dec = NULL;
    }
    player->state = PLAYER_STATE_STOPPED;
}


const char *player_state_name(PlayerState state)
{
    switch (state) {
        case PLAYER_STATE_LOADING:  return "LOADING";
        case PLAYER_STATE_PLAYING:  return "PLAYING";
        case PLAYER_STATE_PAUSED:   return "PAUSED";
        case PLAYER_STATE_STOPPED:  return "STOPPED";
        case PLAYER_STATE_FINISHED: return "FINISHED";
        default:                    return "UNKNOWN";
    }
}
