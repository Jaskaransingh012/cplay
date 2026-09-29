#ifndef CPLAY_METADATA_H
#define CPLAY_METADATA_H


typedef struct {

    char title[128];
    char artist[128];
    char album[128];
    int duration;

} SongMetadata;

int meatadata_read(const char *path, SongMetadata *song_meta);

#endif
