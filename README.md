# cplay

A lightweight command-line music player written in C for Linux. It can scan a local music library, search by title/artist/album, and play MP3 files through the system audio device using `libmpg123` and ALSA.

## Features

- Browse your music library under `~/Music`
- Search tracks by artist, title, or album
- Play MP3 files directly from the command line
- Interactive playback controls in the terminal
- Small, dependency-focused C project suitable for learning audio decoding and playback

## Requirements

Before building the project, make sure the following are installed:

- GCC
- Make
- `libmpg123` development headers
- ALSA development headers

On Debian/Ubuntu-based systems, install them with:

```bash
sudo apt update
sudo apt install build-essential libmpg123-dev libasound2-dev
```

## Build

From the project root:

```bash
make
```

This compiles the project into a binary named `cplay`.

## Usage

Show the CLI help:

```bash
./cplay --help
```

List songs in `~/Music`:

```bash
./cplay --list
```

Search for a song in the local library:

```bash
./cplay search "Believer"
```

Play a specific MP3 file:

```bash
./cplay play ./mexico.mp3
```

You can also play a file from a different directory:

```bash
./cplay play "/path/to/music/song.mp3"
```

## Playback Controls

While a file is playing, the terminal accepts these keys:

- `p` - pause
- `r` - resume
- `q` - quit

## Project Structure

```text
cplay/
├── include/        # Public headers
├── src/           # Source files
├── build/         # Generated object files
├── Makefile       # Build configuration
├── cplay          # Compiled executable
└── README.md      # Project documentation
```
