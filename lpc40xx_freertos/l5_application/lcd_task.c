#include "lcd_task.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "mp3_reader.h"
#include "playback_controls.h"
#include "sd_task.h"

#include "delay.h"

#include "FreeRTOS.h"
#include "queue.h"

/// Set to non-zero to enable debugging, and then you can use MP3__DEBUG_PRINTF()
#define LCD__ENABLE_DEBUGGING 1

#if LCD__ENABLE_DEBUGGING
#define LCD__DEBUG_PRINTF(f_, ...)                                                                                     \
  do {                                                                                                                 \
    fprintf(stderr, "LCD:");                                                                                           \
    fprintf(stderr, (f_), ##__VA_ARGS__);                                                                              \
    fprintf(stderr, "\n");                                                                                             \
  } while (0)
#else
#define LCD__DEBUG_PRINTF(f_, ...) /* NOOP */
#endif

extern xQueueHandle Q_lcd_play_song;
extern xQueueHandle Q_button_pressed;
extern xQueueHandle Q_controls;
extern xQueueHandle Q_songname;

extern int song_name_bytes;

static uint8_t current_line = 0;
static uint8_t current_column = 0;

void lcd__display_main(sd_list_files_s *info) {
  LCD__DEBUG_PRINTF("About to display the menu screen...\n");
  char lcd_text_buffer[20] = {'\0'};
  lcd_clear();
  sprintf(lcd_text_buffer, "Songs (%i):", info->num_of_files);
  lcd_display_string_starting_at(lcd_text_buffer, 0);

  int song_index = 1;
  for (list_node_s *node = info->list_of_file_names; node != NULL && song_index < 4; node = node->next) {
    sprintf(lcd_text_buffer, "%i. %s", song_index, node->file_name);
    lcd_display_string_starting_at(lcd_text_buffer, song_index);
    song_index += 1;
  }
  current_line = 1;

  lcd_display_character_at('<', current_line, 19);
}

void lcd__get_name_of_song(sd_list_files_s *files, char **buffer, int line) {
  int index = 0;
  for (list_node_s *node = files->list_of_file_names; node != NULL; ++index, node = node->next) {
    if (index == line) {
      *buffer = node->file_name;
      break;
    }
  }
}

void lcd_menu_task(void *param) {
  LCD__DEBUG_PRINTF("Task started...\n");
  lcd__initialize();
  lcd_clear();
  char lcd_text_buffer[20] = {'\0'};

  sd_list_files_s info = sd__list_mp3_files();
  lcd__display_main(&info);

  song_state_change_s song_state;
  song_state.songname = NULL;
  song_state.state = STARTED;
  control_button_e action;

  while (1) {
    if (xQueueReceive(Q_lcd_play_song, &song_state, 500)) {
      lcd_clear();
      sprintf(lcd_text_buffer, "> %s", song_state.songname);
      if (song_state.state == STARTED) {
        lcd_display_string_starting_at("Playing:", 0);
      } else if (song_state.state == FINISHED) {
        lcd_display_string_starting_at("Finished playing:", 0);
      }
      lcd_display_string_starting_at(lcd_text_buffer, 1);
      if (song_state.state == FINISHED) {
        // delay__ms(2000);
        // lcd__display_main(&info);
      }
    }

    if (xQueueReceive(Q_controls, &action, 500)) {
      LCD__DEBUG_PRINTF("* Received control: %i\n", action);
      switch (action) {
      case UP: {
        LCD__DEBUG_PRINTF("Moving cursor up\n");
        lcd_remove_character_at(current_line, 19);
        if (current_line > 0) {
          lcd_display_character_at('<', --current_line, 19);
        }

        char *song_to_play_name = NULL;
        lcd__get_name_of_song(&info, &song_to_play_name, current_line - 1);
        if (song_to_play_name != NULL)
          LCD__DEBUG_PRINTF("Name of song: %s\n", song_to_play_name);
      } break;
      case DOWN: {
        LCD__DEBUG_PRINTF("Moving cursor down\n");
        lcd_remove_character_at(current_line, 19);
        if (current_line < lcd_rows)
          lcd_display_character_at('<', ++current_line, 19);

        char *song_to_play_name = NULL;
        lcd__get_name_of_song(&info, &song_to_play_name, current_line - 1);
        if (song_to_play_name != NULL)
          LCD__DEBUG_PRINTF("Name of song: %s\n", song_to_play_name);
      } break;
      case PLAY: {
        LCD__DEBUG_PRINTF("Play\n");
        char *song_to_play_name = NULL;
        lcd__get_name_of_song(&info, &song_to_play_name, current_line - 1);

        LCD__DEBUG_PRINTF("Name of song to play: %s\n", song_to_play_name);
        if (song_to_play_name != NULL) {
          xQueueSend(Q_songname, song_to_play_name, 0);
        }
      } break;
      default:
        break;
      }
    }
  }
  sd__list_cleanup(&info);
}
