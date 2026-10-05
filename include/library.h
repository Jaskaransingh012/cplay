#ifndef CPLAY_LIBRARY_H
#define CPLAY_LIBRARY_H

#include <stddef.h>

typedef struct {
    char *path;
    char *title;
    char *artist;
    char *album;
    int   track;
    int   duration_sec;
} Song;

typedef struct {
    Song  *songs;
    size_t count;
    size_t capacity;
} Library;

void library_init(Library *lib);
int  library_scan(Library *lib, const char *root_dir);
void library_free(Library *lib);
void library_print(const Library *lib);

/* Case-insensitive match on title, artist or album.
 * Returns malloc'd array of pointers INTO lib (free the array only).
 * Empty/NULL query returns NULL with *out_count = 0. */
Song **library_search(const Library *lib, const char *query, size_t *out_count);

#endif
