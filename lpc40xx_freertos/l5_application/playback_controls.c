#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "lpc40xx.h"
#include <stdio.h>

#include "gpio.h"
#include "playback_controls.h"

static gpio_s button0; // P0_0   ->Up
static gpio_s button1; // P0_1   ->Down
static gpio_s button2; // P0_22  ->Play
static gpio_s button3; // P0_18  ->Back

xQueueHandle Q_controls;

void playback_controls__initialize(void) {
  button0 = gpio__construct_with_function(GPIO__PORT_1, 19, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button0);
  LPC_IOCON->P1_19 &= ~(3 << 3);
  LPC_IOCON->P1_19 |= (1 << 3);

  button1 = gpio__construct_with_function(GPIO__PORT_1, 15, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button1);
  LPC_IOCON->P1_15 &= ~(3 << 3);
  LPC_IOCON->P1_15 |= (1 << 3);

  button2 = gpio__construct_with_function(GPIO__PORT_0, 30, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button2);
  // LPC_IOCON->P0_30 &= ~(3 << 3);
  // LPC_IOCON->P1_30 |= (1 << 3);

  button3 = gpio__construct_with_function(GPIO__PORT_0, 29, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button3);
  // LPC_IOCON->P0_29 &= ~(3 << 3);
  // LPC_IOCON->P0_29 |= (1 << 3);
}

void playback_controls_task(void *params) {

  playback_controls__initialize();

  Q_controls = xQueueCreate(1, sizeof(control_button_e));
  control_button_e action;
  while (1) {
    if (gpio__get(button0)) {
      // fprintf(stderr, "Button 0 is pressed!\n");
      action = UP;
      xQueueSend(Q_controls, &action, 0);
    }
    if (gpio__get(button1)) {
      // fprintf(stderr, "Button 1 is pressed!\n");
      action = DOWN;
      xQueueSend(Q_controls, &action, 0);
    }
    if (gpio__get(button2)) {
      // fprintf(stderr, "Button 2 is pressed!\n");
      action = PLAY;
      xQueueSend(Q_controls, &action, 0);
    }
    if (gpio__get(button3)) {
      // fprintf(stderr, "Button 3 is pressed!\n");
      action = BACK;
      xQueueSend(Q_controls, &action, 0);
    }
    vTaskDelay(300);
  }
}
