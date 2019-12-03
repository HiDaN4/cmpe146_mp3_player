#pragma once

enum state_m {
  uninitialized,
  initialized,
  deactivated,
  loading,
  ready,
  playback,
  playMIDIbeep,
  paused_playback,
  testing_memory,
  testing_sinewave,
}; // enum state_m

/// Perform initialization necessary for mp3
void mp3__init(void);

// Reader tasks receives song-name over Q_songname to start reading it
void mp3_reader_task(void *params);

// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *params);