#include "lcd_driver.h"

#include <stdio.h>
#include <string.h>

#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "uart_lab.h"

const uint8_t lcd_rows = 4;
const uint8_t lcd_columns = 20;

static const uint32_t lcd_initial_baud_rate = 9600;

static uint8_t lcd_current_line = 0;
static uint8_t lcd_current_column = 0;

static const uint8_t lcd_command_line_map[4] = {0, 64, 20, 84};

static uart_number_e uart = UART__2;
static const uint8_t uart_tx_pin = 8;

static void configure_uart_pins(void) {
  gpio_s u2_tx = gpio__construct_with_function(GPIO__PORT_2, uart_tx_pin, GPIO__FUNCTION_2);
  gpio__set_as_output(u2_tx);
  LPC_IOCON->P2_8 &= ~(3 << 3); // disable pull up/down resistors

  gpio_s u2_rx = gpio__construct_with_function(GPIO__PORT_2, 9, GPIO__FUNCTION_2);
  gpio__set_as_input(u2_rx);
  LPC_IOCON->P2_9 &= ~(3 << 3); // disable pull up/down resistors
}

static void lcd__update_baud_rate(void) {
  uart_driver__polled_put(uart, 0x7C); // Enter Settings mode
  uart_driver__polled_put(uart, 0x12); // Change baud to 115200bps
}

static void lcd__display_version(void) {
  uart_driver__polled_put(uart, 0x7C);
  uart_driver__polled_put(uart, 0x2C);
}

void lcd__initialize(void) {
  configure_uart_pins();
  uint32_t peripheral_clock_hz = clock__get_peripheral_clock_hz();
  uart_driver__init(uart, peripheral_clock_hz, lcd_initial_baud_rate);
}

void lcd_display_string(char *data) {

  for (int x = 0; data[x] != '\0'; x++) // Send chars until we hit the end of the string
    uart_driver__polled_put(uart, data[x]);
  // fprintf(stderr, "Done printing chars\n");
  int string_length = strlen(data);
  lcd_current_line += string_length / lcd_columns;

  if (lcd_current_line >= lcd_rows)
    lcd_current_line = 0;

  lcd_current_column = string_length % lcd_columns;
}

void lcd_display_string_starting_at(char *data, uint8_t line) {
  lcd_set_cursor(line, 0);
  lcd_display_string(data);
}

void lcd_display_string_at(char *data, uint8_t line, uint8_t column) {
  lcd_set_cursor(line, column);
  lcd_display_string(data);
}

void lcd_set_cursor(uint8_t line, uint8_t column) {
  if (line >= lcd_rows)
    line = 0;
  if (column >= lcd_columns)
    column = 0;

  uart_driver__polled_put(uart, 254);                                       // send command character
  uart_driver__polled_put(uart, 128 + lcd_command_line_map[line] + column); // change position (128) of cursor

  lcd_current_line = line;
  lcd_current_column = column;
}

void lcd_display_character(char character) {
  uart_driver__polled_put(uart, character);

  lcd_current_line += (lcd_current_column + 1) / lcd_columns;

  if (lcd_current_line >= lcd_rows)
    lcd_current_line = 0;

  lcd_current_column = (lcd_current_column + 1) / lcd_columns;
}

void lcd_display_character_at(char character, uint8_t line, uint8_t column) {
  lcd_set_cursor(line, column);
  lcd_display_character(character);
}

void lcd_remove_character_at(uint8_t line, uint8_t column) {
  lcd_set_cursor(line, column);
  uart_driver__polled_put(uart, ' ');
}

void lcd_clear_line(uint8_t line) {
  char empty_line[lcd_columns];
  for (int i = 0; i < lcd_columns - 1; i++) {
    empty_line[i] = ' ';
  }
  empty_line[lcd_columns - 1] = '\0';

  lcd_display_string_starting_at(empty_line, line);
}

void lcd_clear(void) {
  uart_driver__polled_put(uart, '|');
  uart_driver__polled_put(uart, '-');
}