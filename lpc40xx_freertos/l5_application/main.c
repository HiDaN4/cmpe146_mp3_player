#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "sj2_cli.h"

#include "lcd.h"
#include "lcd_task.h"
#include "mp3_reader.h"
#include "playback_controls.h"

// MAIN

int main(void) {
  mp3__init();

  xTaskCreate(mp3_reader_task, "reader_task", (512 * 8) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_player_task, "player_task", (512 * 8) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(lcd_menu_task, "lcd_task", (512 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(playback_controls_task, "controls_task", (512 * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  // xTaskCreate(task, "lcd_task", (512 * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  sj2_cli__init();

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
