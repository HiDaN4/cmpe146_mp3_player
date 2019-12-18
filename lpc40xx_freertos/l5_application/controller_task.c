#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_utils.h"
#include "controller_task.h"
#include "decoder.h"
#include "lcd_driver.h"

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

// EXTERNAL VARIABLES

extern xQueueHandle Q_lcd_play_song;
extern xQueueHandle Q_button_pressed;
extern xQueueHandle Q_controls;
extern xQueueHandle Q_songname;
extern xQueueHandle Q_song_action;

extern int song_name_bytes;

// GLOBAL VARIABLES
static song_state_change_s playing_state;

static uint8_t current_line = 0;

static menu_s *current_menu;
static int volume = 100;

static void lcd__display_main(sd_list_files_s *info, song_state_change_s state);
static void lcd__update_screen(sd_list_files_s *info, song_state_change_s song_state);

static void lcd__get_name_of_song(sd_list_files_s *files, char **buffer, int line);

static void controller_main_menu_handle_button_press(control_button_e button, sd_list_files_s *info);
static void controller_details_menu_handle_button_press(control_button_e button, sd_list_files_s *info);

static void controller_pause_song(void);
static void controller_set_volume(int new_volume);

static void lcd__display_main(sd_list_files_s *info, song_state_change_s state) {
  CONTROLLER__DEBUG_PRINTF("About to display the menu screen...\n");
  char lcd_text_buffer[20] = {'\0'};
  lcd_clear();

  sprintf(lcd_text_buffer, "Songs (%i):", info->num_of_files);
  lcd_display_string_starting_at(lcd_text_buffer, 0);

  if (current_line == 0) {
    current_line = 1;
  }

  if (info->num_of_files > 0) {
    int song_index = 1;
    for (list_node_s *node = info->list_of_file_names; node != NULL && song_index < 4; node = node->next) {
      sprintf(lcd_text_buffer, "%i. %s", song_index, node->file_name);
      lcd_display_string_starting_at(lcd_text_buffer, song_index);

      if (playing_state.songname != NULL && strcmp(playing_state.songname, node->file_name) == 0) {
        // currently playing song
        if (playing_state.state == PLAYING)
          lcd_display_character_at('P', song_index, 17);
        else
          lcd_remove_character_at(song_index, 17);
      }
      song_index += 1;
    }
    lcd_display_character_at('<', current_line, 19);
  } else {
    // no files to show
    lcd_display_string_starting_at("Upload songs to SD", current_line++);
    lcd_display_string_starting_at("card", current_line);
  }
}

static void lcd__get_name_of_song(sd_list_files_s *files, char **buffer, int line) {
  int index = 0;
  for (list_node_s *node = files->list_of_file_names; node != NULL; ++index, node = node->next) {
    if (index == line) {
      *buffer = node->file_name;
      break;
    }
  }
}

static void lcd__update_screen(sd_list_files_s *info, song_state_change_s song_state) {
  lcd_clear();
  switch (song_state.state) {
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

  char lcd_text_buffer[lcd_columns];
  sprintf(lcd_text_buffer, "> %s", song_state.songname);
  lcd_display_string_starting_at(lcd_text_buffer, 1);

  uint8_t volume_line = lcd_rows - 1;
  if (volume != 100 && volume != 0)
    sprintf(lcd_text_buffer, "Volume: %i", volume);
  else if (volume == 100) {
    sprintf(lcd_text_buffer, "Volume: MAX");
  } else if (volume == 0) {
    sprintf(lcd_text_buffer, "Volume: MIN");
  }
  lcd_display_string_starting_at(lcd_text_buffer, volume_line); // display volume on last line
}

void controller_task(void *param) {
  CONTROLLER__DEBUG_PRINTF("Task started...\n");

  lcd__initialize();
  lcd_clear();
  char lcd_text_buffer[lcd_columns];
  memset(lcd_text_buffer, '\0', lcd_columns);

  sd_list_files_s info = sd__list_mp3_files();

  menu_s details_menu = {lcd__update_screen, controller_details_menu_handle_button_press, NULL, NULL};
  menu_s main_menu = {lcd__display_main, controller_main_menu_handle_button_press, NULL, &details_menu};
  details_menu.parent = &main_menu;

  current_menu = &main_menu;
  current_menu->render(&info, playing_state);
  control_button_e action;

  while (1) {
    if (xQueueReceive(Q_lcd_play_song, &playing_state, 500)) {
      CONTROLLER__DEBUG_PRINTF("* Received song state: %i\n", playing_state.state);
      current_menu->render(&info, playing_state);
    }
    if (xQueueReceive(Q_controls, &action, 500)) {
      CONTROLLER__DEBUG_PRINTF("* Received control: %i\n", action);
      current_menu->handler(action, &info);
    }
  }
  sd__list_cleanup(&info);
  vTaskDelete(NULL);
}

static void controller_main_menu_handle_button_press(control_button_e button, sd_list_files_s *info) {
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
    CONTROLLER__DEBUG_PRINTF("Starting playing a song\n");
    char *song_to_play_name = NULL;
    lcd__get_name_of_song(info, &song_to_play_name, current_line - 1);

    CONTROLLER__DEBUG_PRINTF("Name of song to play: %s\n", song_to_play_name);
    if (song_to_play_name != NULL && strcmp(song_to_play_name, playing_state.songname) != 0) {
      current_menu = current_menu->child;
      xQueueSend(Q_songname, song_to_play_name, 0);
    } else {
      // go to the details of the song since it is the same song
      current_menu = current_menu->child;
      current_menu->render(info, playing_state);
    }
  } break;
  default:
    break;
  }
}

static void controller_details_menu_handle_button_press(control_button_e button, sd_list_files_s *info) {
  switch (button) {
  case UP: {
    CONTROLLER__DEBUG_PRINTF("Details up\n");
    controller_set_volume(volume + 20);
    current_menu->render(info, playing_state);
  } break;
  case DOWN: {
    CONTROLLER__DEBUG_PRINTF("Details down\n");
    controller_set_volume(volume - 20);
    current_menu->render(info, playing_state);
  } break;
  case PLAY: {
    CONTROLLER__DEBUG_PRINTF("Play/Pause\n");
    if (playing_state.state == PLAYING) {
      // song is playing, so we need to pause it
      controller_pause_song();
    } else if (playing_state.state != FINISHED) {
      CONTROLLER__DEBUG_PRINTF("Continue playing the song\n");
      playing_state.state = PLAYING;
      current_menu->render(info, playing_state);
      mp3_reader_action_e action = PLAY_SONG;
      xQueueSend(Q_song_action, &action, 0);
    } else if (playing_state.state == FINISHED) {
      // replay the song
      xQueueSend(Q_songname, &playing_state.songname[0], 0);
    }
  } break;
  case BACK: {
    current_menu = current_menu->parent;
    current_menu->render(info, playing_state);
  } break;
  default:
    break;
  }
}

static void controller_pause_song(void) {
  CONTROLLER__DEBUG_PRINTF("Pausing the song\n");
  playing_state.state = PAUSED;
  mp3_reader_action_e action = PAUSE_SONG;
  xQueueSend(Q_song_action, &action, 0);
  current_menu->render(NULL, playing_state);
}

static void controller_set_volume(int new_volume) {
  if (new_volume < 0) {
    new_volume = 0;
  }
  if (new_volume > 100) {
    new_volume = 100;
  }
  int converted_value = convert_volume_value(new_volume);
  CONTROLLER__DEBUG_PRINTF("New volume: %i\n", converted_value);
  mp3_set_volume(converted_value);
  volume = new_volume;
}