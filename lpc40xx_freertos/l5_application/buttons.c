#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>

#include "uart.h"
#include "uart_printf.h"

void ext_led_task(void *task_param);
void ext_switch_task(void *task_param);

static uint32_t ext_button = 0; // pass a value to indicate which button is active

// int main(void) {
//   //  connect P0_6 to the button input and connect P0_10 to the LED output
//   uart0_init();
//   uart_printf__polled(UART__0, "UART 0 init \n");

//   xTaskCreate(int_switch_task, "switch0", (1024 / sizeof(void *)), (void *)&pair_0, PRIORITY_HIGH, NULL);
//   xTaskCreate(int_led_task, "led0", (1024 / sizeof(void *)), (void *)&pair_0, PRIORITY_HIGH, NULL);
//   xTaskCreate(ext_switch_task, "switch1", (1024 / sizeof(void *)), (void *)&ext_button, PRIORITY_HIGH, NULL);
//   xTaskCreate(ext_led_task, "led1", (1024 / sizeof(void *)), (void *)&ext_button, PRIORITY_HIGH, NULL);

//   vTaskStartScheduler();

//   return -1;
// }

void ext_switch_task(void *task_param) {
  /* Get Parameter */
  uint32_t param = (uint32_t)(task_param);

  /* Define Constants Here */
  uint32_t which_button;

  /* Define Local Variables and Objects */
  LPC_IOCON->P0_22 &= ~(7 << 0); // internal switch
  LPC_GPIO0->DIR &= ~(1 << 22);  // input = 0

  LPC_IOCON->P0_18 &= ~(7 << 0); // internal switch
  LPC_GPIO0->DIR &= ~(1 << 18);  // input = 0

  LPC_IOCON->P0_17 &= ~(7 << 0); // internal switch
  LPC_GPIO0->DIR &= ~(1 << 17);  // input = 0

  LPC_IOCON->P0_15 &= ~(7 << 0); // internal switch
  LPC_GPIO0->DIR &= ~(1 << 15);  // input = 0

  /* Initialization Code */
  LPC_GPIOINT->IO0IntEnF |= (1 << 22); // enable falling edge interrupt
  LPC_GPIOINT->IO0IntEnF |= (1 << 18); // enable falling edge interrupt
  LPC_GPIOINT->IO0IntEnF |= (1 << 17); // enable falling edge interrupt
  LPC_GPIOINT->IO0IntEnF |= (1 << 15); // enable falling edge interrupt

  /* Insert Loop Code */
  while (1) {
    which_button = LPC_GPIOINT->IO0IntStatF;
    if ((which_button > 0) && (*((uint32_t *)param) == 0)) // are any of the buttons pressed and not busy
      switch (which_button) {
      case 4194304:
        LPC_GPIOINT->IO0IntClr |= (1 << 22); // clear interrupt status
        ext_button = 1;
        break;
      case 262144:
        LPC_GPIOINT->IO0IntClr |= (1 << 18); // clear interrupt status
        ext_button = 2;
        break;
      case 131072:
        LPC_GPIOINT->IO0IntClr |= (1 << 17); // clear interrupt status
        ext_button = 3;
        break;
      case 32768:
        LPC_GPIOINT->IO0IntClr |= (1 << 15); // clear interrupt status
        ext_button = 4;
        break;
      }
  }
  vTaskDelay(100);
}

void ext_led_task(void *task_param) {
  /* Get Parameter */
  uint32_t param = (uint32_t)(task_param);

  /* Define Constants Here */

  /* Define Local Variables and Objects */

  /* Initialization Code */

  /* Insert Loop Code */
  while (1) {
    switch (*((uint32_t *)param)) {
    case 1:
      uart_printf__polled(UART__0, "pressed up\n");
      break;
    case 2:
      uart_printf__polled(UART__0, "pressed down\n");
      break;
    case 3:
      uart_printf__polled(UART__0, "pressed play\n");
      break;
    case 4:
      uart_printf__polled(UART__0, "pressed stop\n");
      break;
    }
    ext_button = 0;
    vTaskDelay(100);
  }
}
