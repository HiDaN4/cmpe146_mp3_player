#include "lpc40xx.h"
#include <stdio.h>

#include "clock.h"
#include "gpio.h"
#include "lpc_peripherals.h"
#include "ssp2.h"
#include "uart_lab.h"

static const uint32_t lcd_initial_baud_rate = 9600;
static const uint32_t baud_rate = 115200;

static const uint8_t lcd_rows = 4;
static const uint8_t lcd_columns = 20;

static uart_number_e uart = UART__2;

void configure_uart_pins(void) {
  gpio_s u2_tx = gpio__construct_with_function(GPIO__PORT_2, 8, GPIO__FUNCTION_2);
  gpio__set_as_output(u2_tx);
  LPC_IOCON->P2_8 &= ~(3 << 3); // disable pull up/down resistors

  gpio_s u2_rx = gpio__construct_with_function(GPIO__PORT_2, 9, GPIO__FUNCTION_2);
  gpio__set_as_input(u2_rx);
  LPC_IOCON->P2_9 &= ~(3 << 3); // disable pull up/down resistors
}

void lcd__update_baud_rate(void) {
  uart_lab__polled_put(uart, 0x7C); // Enter Settings mode
  uart_lab__polled_put(uart, 0x12); // Change baud to 115200bps
}

void lcd__display_version(void) {
  uart_lab__polled_put(uart, 0x7C);
  uart_lab__polled_put(uart, 0x2C);
}

void lcd__initialize(void) {
  configure_uart_pins();
  uart_lab__init(uart, clock__get_peripheral_clock_hz(), lcd_initial_baud_rate);
}

void lcd_display_string(char *data) {
  fprintf(stderr, "Printing chars\n");

  for (int x = 0; data[x] != '\0'; x++) // Send chars until we hit the end of the string
    uart_lab__polled_put(uart, data[x]);
  fprintf(stderr, "Done printing chars\n");
}

void lcd_clear(void) {
  uart_lab__polled_put(uart, '|');
  uart_lab__polled_put(uart, '-');
}