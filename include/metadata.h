#ifndef CPLAY_METADATA_H
#define CPLAY_METADATA_H

#include <stddef.h>

typedef struct {

    char title[128];
    char artist[128];
    char album[128];
    int track;
    int duration_sec;

} Metadata;

int metadata_read(const char *path, Metadata *song_meta);

void metadata_format_duration(int seconds, char *out, size_t size);


#endif
