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
#include "mp3.h"

static void task(void *params) {
  lcd__initialize();
  fprintf(stderr, "Initialized LCD...\n");

  int cycles = 0;
  vTaskDelay(500); // wait 500ms until LCD is done displaying Splash Screen
  while (1) {
    fprintf(stderr, "Clearing LCD...\n");
    lcd_clear();
    vTaskDelay(250);

    char tempString[50]; // Needs to be large enough to hold the entire string with up to 5 digits
    sprintf(tempString, "Cycles: %d ", cycles++);

    fprintf(stderr, "Displaying string on LCD...\n");
    lcd_display_string(tempString);

    vTaskDelay(250);
  }
}

// MAIN

int main(void) {
  mp3__init();

  xTaskCreate(mp3_reader_task, "reader_task", (512 * 8) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_player_task, "player_task", (512 * 8) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  // xTaskCreate(task, "lcd_task", (512 * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  sj2_cli__init();

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
