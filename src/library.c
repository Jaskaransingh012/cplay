#define _POSIX_C_SOURCE 200809L
#ifndef PATH_MAX


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#include "library.h"

#define PATH_MAX 4096
#define INITIAL_CAPACITY 16

void library_init(Library *lib) {

    lib->songs = NULL;
    lib->count = 0;
    lib->capacity = 0;

}

static int has_wav_extension(const char *fileName) {

    size_t len = strlen(fileName);
    // .wav -> len 4
    if(len<4) return 0;

    const char *ext = fileName + len - 4;

     return tolower((unsigned char)ext[0]) == '.' &&
           tolower((unsigned char)ext[1]) == 'w' &&
           tolower((unsigned char)ext[2]) == 'a' &&
           tolower((unsigned char)ext[3]) == 'v';

}

static char *derive_title(const char *basename) {

    char *title = strdup(basename);
    if(title==NULL) return NULL;

    char *dot = strrchr(title, '.');
    if(dot != NULL) *dot = '\0';

    return title;

}

static int library_ensure_capacity(Library *lib) {

    if(lib->count < lib->capacity) return 0;
    size_t new_capacity = (lib->capacity == 0) ? INITIAL_CAPACITY : lib->capacity * 2;

    Song *new_songs = realloc(lib->songs, new_capacity * sizeof(Song));
    if(new_songs == NULL) {
        return -1;
    }

    lib->songs = new_songs;
    lib->capacity = new_capacity;
    return 0;

}

static int library_add_song(Library *lib, const char *full_path, const char *basename) {

    if(library_ensure_capacity(lib) != 0) {
        fprintf(stderr, "Error: out of memory while scanning library\n");
        return -1;
    }

    Song *song = &lib->songs[lib->count];
    song->path = strdup(full_path);
    song->title = derive_title(basename);
    song->artist = strdup("Unknow Artist");
    song->duration = 0;

    if (song->path == NULL || song->title == NULL || song->artist == NULL) {
        fprintf(stderr, "Error: out of memory while adding song '%s'\n", full_path);
        free(song->path);
        free(song->title);
        free(song->artist);
        return -1;
    }

    lib->count++;
    return 0;

}

static void scan_dir(Library *lib, const char *dir_path) {

    DIR *dir = opendir(dir_path);

    if(dir==NULL) {
        fprintf(stderr, "Warning: cannot open directory '%s': %s\n", dir_path, strerror(errno));
        return;
    }

    struct dirent *entry;

    while((entry = readdir(dir)) != NULL) {

        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[PATH_MAX];

        int written = snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (written < 0 || (size_t)written >= sizeof(full_path)) {
            fprintf(stderr, "Warning: path too long, skipping '%s/%s'\n", dir_path, entry->d_name);
            continue;
        }

        struct stat st;

        if (lstat(full_path, &st) != 0) {
            fprintf(stderr, "Warning: cannot stat '%s': %s\n", full_path, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            scan_dir(lib, full_path); /* recurse */
        }
         else if (S_ISREG(st.st_mode) && has_wav_extension(entry->d_name)) {
            library_add_song(lib, full_path, entry->d_name);
        }

    }
    closedir(dir);
}

int library_scan(Library *lib, const char *root_dir)
{

    struct stat st;

    if (stat(root_dir, &st) != 0) {
        fprintf(stderr, "Error: music directory not found: %s\n", root_dir);
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: not a directory: %s\n", root_dir);
        return -1;
    }

    scan_dir(lib, root_dir);
    return 0;

}

void library_free(Library *lib)
{

    for (size_t i = 0; i < lib->count; i++) {
        free(lib->songs[i].path);
        free(lib->songs[i].title);
        free(lib->songs[i].artist);
    }
    free(lib->songs);
    lib->songs = NULL;
    lib->count = 0;
    lib->capacity = 0;

}

void library_print(const Library *lib)
{

    if (lib->count == 0) {
        printf("No songs found.\n");
        return;
    }
    printf("Found %zu song(s):\n", lib->count);
    for (size_t i = 0; i < lib->count; i++) {
        printf("  %zu. %s - %s\n", i + 1, lib->songs[i].artist, lib->songs[i].title);
    }

}

Song **library_search(const Library *lib, const char *query, size_t *out_count)
{

    *out_count = 0;
    if (lib->count == 0) {
        return NULL;
    }

    Song **matches = malloc(lib->count * sizeof(Song *));
    if (matches == NULL) {
        return NULL;
    }

    size_t n = 0;
    for (size_t i = 0; i < lib->count; i++) {
        if (strstr(lib->songs[i].title, query) != NULL) {
            matches[n++] = &lib->songs[i];
        }
    }

    if (n == 0) {
        free(matches);
        return NULL;
    }

    *out_count = n;
    return matches;

}


#endif
