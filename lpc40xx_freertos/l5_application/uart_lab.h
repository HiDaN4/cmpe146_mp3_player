#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  UART__2 = 2,
  UART__3 = 3,
} uart_number_e;

void uart_driver__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);
void uart_driver__set_baud_rate(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);

bool uart_driver__polled_get(uart_number_e uart, char *input_byte);

bool uart_driver__polled_put(uart_number_e uart, char output_byte);

void uart__enable_receive_interrupt(uart_number_e uart_number);
bool uart_driver__get_char_from_queue(char *byte, uint32_t timeout);