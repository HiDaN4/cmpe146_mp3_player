#include "lcd_task.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "mp3.h"
#include "sd_task.h"

#include "delay.h"

#include "FreeRTOS.h"
#include "queue.h"

extern xQueueHandle Q_lcd_play_song;
extern xQueueHandle Q_button_pressed;
extern int song_name_bytes;

void lcd__display_main(sd_list_files_s *info) {
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

  lcd_display_character_at('<', 1, 19);
}

void lcd_menu_task(void *param) {
  lcd__initialize();
  lcd_clear();
  char lcd_text_buffer[20] = {'\0'};

  sd_list_files_s info = sd__list_mp3_files();
  lcd__display_main(&info);

  song_state_change_s song_state;
  song_state.songname = NULL;
  song_state.state = STARTED;

  while (1) {
    if (xQueueReceive(Q_lcd_play_song, &song_state, portMAX_DELAY)) {
      lcd_clear();
      sprintf(lcd_text_buffer, "> %s", song_state.songname);
      if (song_state.state == STARTED) {
        lcd_display_string_starting_at("Playing:", 0);
      } else if (song_state.state == FINISHED) {
        lcd_display_string_starting_at("Finished playing:", 0);
      }
      lcd_display_string_starting_at(lcd_text_buffer, 1);
      if (song_state.state == FINISHED) {
        delay__ms(2000);
        lcd__display_main(&info);
      }
    }
  }
  sd__list_cleanup(&info);
}
