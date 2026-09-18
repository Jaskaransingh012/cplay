#ifndef CPLAY_LIBRARY_H
#define CPLAY_LIBTRARY_H

#include <stddef.h>

typedef struct {
    char *path;
    char *title;
    char *artist;
    int duration;
} Song;

typedef struct {
    Song *songs;
    size_t count;
    size_t capacity;
} Library;


void library_init(Library *lib);

int library_scan(Library *lib, const char* root_dir);

void library_free(Library *lib);
void library_print(const Library *lib);


Song **library_search(const Library *lib, const char *query, size_t *out_count);

#endif
