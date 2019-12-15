#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "controller_task.h"
#include "lcd_driver.h"
#include "mp3_reader.h"
#include "playback_controls.h"
#include "sd_task.h"

#include "FreeRTOS.h"
#include "delay.h"
#include "queue.h"

/// Set to non-zero to enable debugging, and then you can use MP3__DEBUG_PRINTF()
#define CONTROLLER__ENABLE_DEBUGGING 1

#if CONTROLLER__ENABLE_DEBUGGING
#define CONTROLLER__DEBUG_PRINTF(f_, ...)                                                                              \
  do {                                                                                                                 \
    fprintf(stderr, "LCD:");                                                                                           \
    fprintf(stderr, (f_), ##__VA_ARGS__);                                                                              \
    fprintf(stderr, "\n");                                                                                             \
  } while (0)
#else
#define CONTROLLER__DEBUG_PRINTF(f_, ...) /* NOOP */
#endif

extern xQueueHandle Q_lcd_play_song;
extern xQueueHandle Q_button_pressed;
extern xQueueHandle Q_controls;
extern xQueueHandle Q_songname;

extern int song_name_bytes;
extern song_state_change_s playing_state;

static uint8_t current_line = 0;
static uint8_t current_column = 0;

static lcd_menu_screen_e lcd_menu_screen;

static void controller_handle_button_press(control_button_e button, sd_list_files_s *info);
static void controller_pause_song(void);

bool lcd_is_playing(void) { return playing_state.state == PLAYING; }

void lcd__display_main(sd_list_files_s *info) {
  CONTROLLER__DEBUG_PRINTF("About to display the menu screen...\n");
  lcd_menu_screen = MAIN;
  char lcd_text_buffer[20] = {'\0'};
  lcd_clear();

  sprintf(lcd_text_buffer, "Songs (%i):", info->num_of_files);
  lcd_display_string_starting_at(lcd_text_buffer, 0);

  current_line = 1;
  if (info->num_of_files > 0) {
    int song_index = 1;
    for (list_node_s *node = info->list_of_file_names; node != NULL && song_index < 4; node = node->next) {
      sprintf(lcd_text_buffer, "%i. %s", song_index, node->file_name);
      lcd_display_string_starting_at(lcd_text_buffer, song_index);
      song_index += 1;
    }
    lcd_display_character_at('<', current_line, 19);
  } else {
    // no files to show
    lcd_display_string_starting_at("Upload songs to SD", current_line++);
    lcd_display_string_starting_at("card", current_line);
  }
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

void lcd__update_screen(mp3_reader_state_e state) {
  if (lcd_menu_screen != DETAILS) {
    return;
  }

  switch (state) {
  case PLAYING:
    lcd_display_string_starting_at("Playing:", 0);
    break;
  case PAUSED:
    lcd_display_string_starting_at("Paused:", 0);
    break;
  case FINISHED:
    lcd_display_string_starting_at("Finished playing:", 0);
    break;
  default:
    break;
  }
}

void controller_task(void *param) {
  CONTROLLER__DEBUG_PRINTF("Task started...\n");
  lcd__initialize();
  lcd_clear();
  char lcd_text_buffer[20] = {'\0'};

  sd_list_files_s info = sd__list_mp3_files();
  lcd__display_main(&info);

  song_state_change_s song_state;
  song_state.songname = NULL;
  song_state.state = IDLE;
  control_button_e action;

  while (1) {
    if (xQueueReceive(Q_lcd_play_song, &song_state, 500)) {
      CONTROLLER__DEBUG_PRINTF("* Received song state: %i\n", song_state.state);
      lcd_clear();
      sprintf(lcd_text_buffer, "> %s", song_state.songname);
      lcd__update_screen(song_state.state);
      lcd_display_string_starting_at(lcd_text_buffer, 1);
    }

    if (xQueueReceive(Q_controls, &action, 500)) {
      CONTROLLER__DEBUG_PRINTF("* Received control: %i\n", action);
      controller_handle_button_press(action, &info);
    }
  }
  sd__list_cleanup(&info);
  vTaskDelete(NULL);
}

static void controller_handle_button_press(control_button_e button, sd_list_files_s *info) {
  switch (button) {
  case UP: {
    CONTROLLER__DEBUG_PRINTF("Moving cursor up\n");
    if (current_line > 1) {
      lcd_remove_character_at(current_line, 19);
      lcd_display_character_at('<', --current_line, 19);
    }

    char *song_to_play_name = NULL;
    lcd__get_name_of_song(info, &song_to_play_name, current_line - 1);
    if (song_to_play_name != NULL)
      CONTROLLER__DEBUG_PRINTF("Name of song: %s\n", song_to_play_name);
  } break;
  case DOWN: {
    CONTROLLER__DEBUG_PRINTF("Moving cursor down\n");
    if (current_line < info->num_of_files) {
      lcd_remove_character_at(current_line, 19);
      lcd_display_character_at('<', ++current_line, 19);
    }

    char *song_to_play_name = NULL;
    lcd__get_name_of_song(info, &song_to_play_name, current_line - 1);
    if (song_to_play_name != NULL)
      CONTROLLER__DEBUG_PRINTF("Name of song: %s\n", song_to_play_name);
  } break;
  case PLAY: {
    CONTROLLER__DEBUG_PRINTF("Play/Pause\n");

    switch (lcd_menu_screen) {
      case MENU:
      {
        
      }
      break;
      case DETAILS:
      {
        // can only pause/play
      }
      break;
      default: break;
    }

    if (playing_state.state != PLAYING) {
      
      // nothing is playing, so we need to start playing a song
      if (playing_state.state == IDLE || playing_state.state == FINISHED) {
        CONTROLLER__DEBUG_PRINTF("Starting playing a song\n");
        char *song_to_play_name = NULL;
        lcd__get_name_of_song(info, &song_to_play_name, current_line - 1);

        CONTROLLER__DEBUG_PRINTF("Name of song to play: %s\n", song_to_play_name);
        if (song_to_play_name != NULL) {
          xQueueSend(Q_songname, song_to_play_name, 0);
        }
      } else {
        CONTROLLER__DEBUG_PRINTF("Continue playing the song\n");
        playing_state.state = PLAYING;
        lcd__update_screen(PLAYING);
      }
    } else if (playing_state.state == PLAYING) {
      // song is playing, so we need to pause it
      controller_pause_song();
    }
  } break;
  case BACK: {
    switch (lcd_menu_screen) {
    case DETAILS: {
      lcd_menu_screen = MAIN;
      lcd__display_main(info);
    } break;
    default:
      break;
    }
  } break;
  default:
    break;
  }
}

static void controller_pause_song(void) {
  CONTROLLER__DEBUG_PRINTF("Pausing the song\n");
  playing_state.state = PAUSED;
  lcd__update_screen(PAUSED);
}