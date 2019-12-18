#pragma once
#include <stdbool.h>

#include "mp3_reader.h"
#include "playback_controls.h"
#include "sd_task.h"

typedef enum lcd_menu_screen { MAIN, DETAILS } lcd_menu_screen_e;

typedef enum mp3_reader_action { PLAY_SONG, PAUSE_SONG } mp3_reader_action_e;

void controller_task(void *params);

typedef struct menu {
  void (*render)(sd_list_files_s *info, song_state_change_s state);
  void (*handler)(control_button_e button, sd_list_files_s *info);
  struct menu *parent;
  struct menu *child;
} menu_s;