#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include "library.h"
#include "metadata.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define INITIAL_CAPACITY 16

void library_init(Library *lib)
{
    lib->songs = NULL;
    lib->count = 0;
    lib->capacity = 0;
}

static int ends_with_ignore_case(const char *s, const char *ext)
{
    size_t sl = strlen(s);
    size_t el = strlen(ext);
    if (sl < el) return 0;
    for (size_t i = 0; i < el; i++) {
        if (tolower((unsigned char)s[sl - el + i]) != tolower((unsigned char)ext[i]))
            return 0;
    }
    return 1;
}

static int is_audio_file(const char *name)
{
    return ends_with_ignore_case(name, ".wav") || ends_with_ignore_case(name, ".mp3");
}

/* "Believer.mp3" -> "Believer" (caller frees) */
static char *title_from_filename(const char *basename)
{
    char *title = strdup(basename);
    if (title == NULL) return NULL;
    char *dot = strrchr(title, '.');
    if (dot != NULL) *dot = '\0';
    return title;
}

static int library_ensure_capacity(Library *lib)
{
    if (lib->count < lib->capacity) return 0;
    size_t new_capacity = (lib->capacity == 0) ? INITIAL_CAPACITY : lib->capacity * 2;
    Song *new_songs = realloc(lib->songs, new_capacity * sizeof(Song));
    if (new_songs == NULL) return -1;
    lib->songs = new_songs;
    lib->capacity = new_capacity;
    return 0;
}

static int library_add_song(Library *lib, const char *full_path, const char *basename)
{
    if (library_ensure_capacity(lib) != 0) {
        fprintf(stderr, "Error: out of memory while scanning library\n");
        return -1;
    }

    Metadata meta;
    metadata_read(full_path, &meta);

    Song *song = &lib->songs[lib->count];

    song->path   = strdup(full_path);
    song->title  = (meta.title[0]  != '\0') ? strdup(meta.title)  : title_from_filename(basename);
    song->artist = strdup(meta.artist[0] != '\0' ? meta.artist : "Unknown Artist");
    song->album  = strdup(meta.album[0]  != '\0' ? meta.album  : "Unknown Album");
    song->track  = meta.track;
    song->duration_sec = meta.duration_sec;

    if (song->path == NULL || song->title == NULL ||
        song->artist == NULL || song->album == NULL) {
        fprintf(stderr, "Error: out of memory while adding '%s'\n", full_path);
        free(song->path);
        free(song->title);
        free(song->artist);
        free(song->album);
        return -1;
    }

    lib->count++;
    return 0;
}

static void scan_dir(Library *lib, const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "Warning: cannot open directory '%s': %s\n", dir_path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char full_path[PATH_MAX];
        int written = snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(full_path)) continue;

        struct stat st;
        if (lstat(full_path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_dir(lib, full_path);
        } else if (S_ISREG(st.st_mode) && is_audio_file(entry->d_name)) {
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
        free(lib->songs[i].album);
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
        char dur[16];
        metadata_format_duration(lib->songs[i].duration_sec, dur, sizeof(dur));
        printf("  %zu. %s - %s  [%s]  (%s)\n", i + 1,
               lib->songs[i].artist, lib->songs[i].title, dur, lib->songs[i].album);
    }
}

/* ---------- search ---------- */

static int contains_ignore_case(const char *haystack, const char *needle)
{
    size_t nlen = strlen(needle);
    if (nlen == 0) return 1;

    for (const char *h = haystack; *h != '\0'; h++) {
        size_t i = 0;
        while (i < nlen && h[i] != '\0' &&
               tolower((unsigned char)h[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == nlen) return 1;
    }
    return 0;
}

Song **library_search(const Library *lib, const char *query, size_t *out_count)
{
    *out_count = 0;
    if (query == NULL || query[0] == '\0') return NULL;
    if (lib->count == 0) return NULL;

    Song **matches = malloc(lib->count * sizeof(Song *));
    if (matches == NULL) return NULL;

    size_t n = 0;
    for (size_t i = 0; i < lib->count; i++) {
        const Song *s = &lib->songs[i];
        if (contains_ignore_case(s->title, query)  ||
            contains_ignore_case(s->artist, query) ||
            contains_ignore_case(s->album, query)) {
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
