#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "sj2_cli.h"

//
#include "controller_task.h"
#include "decoder.h"
#include "mp3_reader.h"
#include "playback_controls.h"

// MAIN

int main(void) {
  mp3__init();

  xTaskCreate(mp3_player_task, "player_task", (512 * 8) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(mp3_reader_task, "reader_task", (512 * 8) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(controller_task, "controller_task", (512 * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(playback_controls_task, "controls_task", (512 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  // sj2_cli__init();
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
