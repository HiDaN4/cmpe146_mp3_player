#include "mp3_reader.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "ff.h"
#include "queue.h"

#include "lcd.h"

#include "sd_task.h"

const int song_name_bytes = 32;
typedef char songname[32];

extern const int data_size_bytes;

xQueueHandle Q_songdata;
xQueueHandle Q_songname;
xQueueHandle Q_lcd_play_song;

/// Open given file name and return true on success
bool open_file(FIL *file, char *name) {
  FRESULT result = f_open(file, name, (FA_READ | FA_OPEN_EXISTING));

  return (FR_OK == result);
}

/// Close file pointer
void close_file(FIL *file) { f_close(file); }

/// Read bytes from file
bool read_bytes(FIL *file, char *buffer, int buffer_size) {
  UINT bytes_read = 0;
  FRESULT result = f_read(file, buffer, (UINT)buffer_size, &bytes_read);

  if (FR_OK == result) {
    // success reading bytes
    // printf("READ: Successfully read num of bytes: %u\n", bytes_read);
    return true;
  } else {
    printf("READ: Error reading data\n");
    return false;
  }
}

/// Perform initialization necessary for mp3
void mp3__init() {
  Q_songname = xQueueCreate(2, sizeof(songname));
  Q_songdata = xQueueCreate(1, sizeof(char) * data_size_bytes);
  Q_lcd_play_song = xQueueCreate(2, sizeof(song_state_change_s));
}

// Reader tasks receives song-name over Q_songname to start reading it
void mp3_reader_task(void *params) {
  char name[song_name_bytes];
  char data[data_size_bytes];
  FIL file;

  // initialize data to 0
  memset(data, 0, data_size_bytes);
  memset(name, 0, song_name_bytes);

  while (1) {
    xQueueReceive(Q_songname, &name[0], portMAX_DELAY);
    printf("Received song to play: %s\n", name);

    if (open_file(&file, name) == false) {
      printf("ERROR: Failed to open file named: %s\n", name);
      continue;
    }

    song_state_change_s play_state;
    play_state.songname = name;
    play_state.state = STARTED;

    // signal to LCD about playing song
    xQueueSend(Q_lcd_play_song, &play_state, 0);

    while (f_eof(&file) == false) {
      if (read_bytes(&file, data, data_size_bytes)) {
        xQueueSend(Q_songdata, &data[0], portMAX_DELAY);
      } else {
        // error reading bytes
        printf("ERROR: Failed to read bytes from the file!\n");
        break;
      }
      // check if q_songs has more items? if yes, break from this loop
      if (uxQueueMessagesWaiting(Q_songname) > 0) {
        break;
      }
    }
    close_file(&file);

    play_state.state = FINISHED;
    xQueueSend(Q_lcd_play_song, &play_state, 10);
  }
}