#pragma once

typedef enum MP3_READER_STATE { IDLE, PLAYING, PAUSED, FINISHED } mp3_reader_state_e;

typedef struct SONG_STATE_CHANGE {
  char *songname;
  mp3_reader_state_e state;
} song_state_change_s;

/// Perform initialization necessary for mp3
void mp3__init(void);

// Reader tasks receives song-name over Q_songname to start reading it
void mp3_reader_task(void *params);