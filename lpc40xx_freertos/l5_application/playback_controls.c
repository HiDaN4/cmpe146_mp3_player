#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "lpc40xx.h"

//#include <stdbool.h>
//#include <stdint.h>
#include <stdio.h>

#include "gpio.h"
#include "playback_controls.h"
//#include "uart.h"
//#include "uart_printf.h"

static gpio_s button0; // P0_0   ->Up
static gpio_s button1; // P0_1   ->Down
static gpio_s button2; // P0_22  ->Play
static gpio_s button3; // P0_18  ->Back
static gpio_s button4; // P0_17  ->Vol_up
static gpio_s button5; // P0_15  ->Vol_down

xQueueHandle Q_controls;

void playback_controls__initialize(void) {
  button0 = gpio__construct_with_function(GPIO__PORT_0, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button0);
  LPC_IOCON->P0_0 &= ~(3 << 3);
  LPC_IOCON->P0_0 |= (1 << 3);

  button1 = gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button1);
  LPC_IOCON->P0_1 &= ~(3 << 3);
  LPC_IOCON->P0_1 |= (1 << 3);

  button2 = gpio__construct_with_function(GPIO__PORT_0, 22, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button2);
  LPC_IOCON->P0_22 &= ~(3 << 3);
  LPC_IOCON->P0_22 |= (1 << 3);

  button3 = gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button3);
  LPC_IOCON->P0_18 &= ~(3 << 3);
  LPC_IOCON->P0_18 |= (1 << 3);

  button4 = gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button4);
  LPC_IOCON->P0_17 &= ~(3 << 3);
  LPC_IOCON->P0_17 |= (1 << 3);

  button5 = gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(button5);
  LPC_IOCON->P0_15 &= ~(3 << 3);
  LPC_IOCON->P0_15 |= (1 << 3);
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
    if (gpio__get(button4)) {
      // fprintf(stderr, "Button 4 is pressed!\n");
      action = VOL_UP;
      xQueueSend(Q_controls, &action, 0);
    }
    if (gpio__get(button5)) {
      // fprintf(stderr, "Button 5 is pressed!\n");
      action = VOL_DOWN;
      xQueueSend(Q_controls, &action, 0);
    }
    vTaskDelay(300);
  }
}
