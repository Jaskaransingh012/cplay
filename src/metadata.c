#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <mpg123.h>
#include "metadata.h"
#include "wav.h"

static int has_ext(const char *path, const char *ext)
{
    size_t pl = strlen(path);
    size_t el = strlen(ext);
    if (pl < el) return 0;
    for (size_t i = 0; i < el; i++) {
        if (tolower((unsigned char)path[pl - el + i]) != tolower((unsigned char)ext[i]))
            return 0;
    }
    return 1;
}

/* copy an mpg123_string into a fixed buffer (safe if NULL/empty) */
static void copy_mpg_string(char *dst, size_t size, const mpg123_string *s)
{
    if (s != NULL && s->p != NULL && s->fill > 1) {
        snprintf(dst, size, "%s", s->p);
    }
}

static void read_mp3(const char *path, Metadata *m)
{
    int err;
    mpg123_handle *h = mpg123_new(NULL, &err);
    if (h == NULL) return;

    if (mpg123_open(h, path) != MPG123_OK) {
        mpg123_delete(h);
        return;
    }

    long rate;
    int channels, encoding;
    if (mpg123_getformat(h, &rate, &channels, &encoding) == MPG123_OK && rate > 0) {
        mpg123_scan(h);                       /* reads whole file: accurate length + all tags */
        off_t samples = mpg123_length(h);     /* samples per channel */
        if (samples > 0) m->duration_sec = (int)(samples / rate);
    }

    mpg123_id3v1 *v1 = NULL;
    mpg123_id3v2 *v2 = NULL;
    if (mpg123_id3(h, &v1, &v2) == MPG123_OK) {
        /* ID3v1 first (old, fixed 30-byte fields, not NUL-terminated) */
        if (v1 != NULL) {
            snprintf(m->title,  sizeof(m->title),  "%.30s", v1->title);
            snprintf(m->artist, sizeof(m->artist), "%.30s", v1->artist);
            snprintf(m->album,  sizeof(m->album),  "%.30s", v1->album);
        }
        /* ID3v2 overrides v1 when present (newer, better) */
        if (v2 != NULL) {
            copy_mpg_string(m->title,  sizeof(m->title),  v2->title);
            copy_mpg_string(m->artist, sizeof(m->artist), v2->artist);
            copy_mpg_string(m->album,  sizeof(m->album),  v2->album);

            for (size_t i = 0; i < v2->texts; i++) {
                if (memcmp(v2->text[i].id, "TRCK", 4) == 0 && v2->text[i].text.p != NULL) {
                    m->track = atoi(v2->text[i].text.p);   /* handles "5/12" -> 5 */
                }
            }
        }
    }

    mpg123_close(h);
    mpg123_delete(h);
}

static void read_wav(const char *path, Metadata *m)
{
    WavFile wav;
    if (wav_open(&wav, path) != 0) return;

    long bytes_per_sec = (long)wav.sample_rate * wav.channels * (wav.bits_per_sample / 8);
    if (bytes_per_sec > 0) {
        m->duration_sec = (int)(wav.data_size / (unsigned long)bytes_per_sec);
    }
    wav_close(&wav);
}

int metadata_read(const char *path, Metadata *meta)
{
    memset(meta, 0, sizeof(*meta));

    if (has_ext(path, ".mp3")) {
        read_mp3(path, meta);
        return 0;
    }
    if (has_ext(path, ".wav")) {
        read_wav(path, meta);
        return 0;
    }
    return -1;
}

void metadata_format_duration(int seconds, char *out, size_t size)
{
    if (seconds < 0) seconds = 0;
    snprintf(out, size, "%02d:%02d", seconds / 60, seconds % 60);
}
