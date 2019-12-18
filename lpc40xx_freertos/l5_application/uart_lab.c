#include "uart_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"

static const uint32_t uart2_power_bit = (1 << 24);
static const uint32_t uart3_power_bit = (1 << 25);

static LPC_UART_TypeDef *uarts[] = {LPC_UART2, LPC_UART2, LPC_UART2, LPC_UART3}; // index 0-2 for UART2

static QueueHandle_t uart_rx_queue;

void uart__enable_dll_dlm(uart_number_e uart) { uarts[uart]->LCR |= (1 << 7); }

void uart__disable_dll_dlm(uart_number_e uart) { uarts[uart]->LCR &= ~(1 << 7); }

void uart_driver__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  switch (uart) {
  case UART__2:
    LPC_SC->PCONP |= uart2_power_bit;
    break;
  case UART__3:
    LPC_SC->PCONP |= uart3_power_bit;
    break;
  default:
    // do nothing
    return;
  }
  uarts[uart]->LCR = (3 << 0); // set 8-bit character length

  uart_driver__set_baud_rate(uart, peripheral_clock, baud_rate);
}

void uart_driver__set_baud_rate(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  uart__enable_dll_dlm(uart);
  {
    const uint32_t register_value = (peripheral_clock) / (16 * baud_rate);
    uarts[uart]->DLL = register_value & 0xFF;
    uarts[uart]->DLM = (register_value >> 8) & 0xFF;
  }
  uart__disable_dll_dlm(uart);

  uarts[uart]->FDR &= ~(7 << 0); // disable fractional divider
}

bool uart_driver__polled_get(uart_number_e uart, char *input_byte) {
  while (!(uarts[uart]->LSR & 1)) {
    ; // wait while it does not become 1
  }
  *input_byte = uarts[uart]->RBR;
  return true;
}

bool uart_driver__polled_put(uart_number_e uart, char output_byte) {
  while (!(uarts[uart]->LSR & (1 << 5))) {
    ; // wait until there is something to write
  }
  uarts[uart]->THR |= output_byte;
  return true;
}

static void uart_isr(void) {

  LPC_UART_TypeDef *uart_with_interrupt = NULL;
  if ((uarts[UART__2]->IIR & (1 << 0)) == 0) {
    uart_with_interrupt = uarts[UART__2];
  } else if ((uarts[UART__3]->IIR & (1 << 0)) == 0) {
    uart_with_interrupt = uarts[UART__3];
  }

  if (uart_with_interrupt == NULL) {
    fprintf(stderr, "Undefined UART issued the interrupt!\n");
    return;
  }

  if (!(uart_with_interrupt->LSR & (1 << 0))) {
    fprintf(stderr, "No data to read from UART in the interrupt!\n");
    return;
  }

  const char byte = uart_with_interrupt->RBR;
  xQueueSendFromISR(uart_rx_queue, &byte, NULL);
}

void uart__enable_receive_interrupt(uart_number_e uart_number) {
  switch (uart_number) {
  case UART__2:
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, uart_isr);
    break;
  case UART__3:
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, uart_isr);
    break;
  default:
    return;
  }

  uart__disable_dll_dlm(uart_number);

  uarts[uart_number]->IER |= (1 << 0); // enable RDA interrupt

  if (uart_rx_queue == NULL) {
    uart_rx_queue = xQueueCreate(10, sizeof(uint8_t));
  }
}

bool uart_driver__get_char_from_queue(char *byte, uint32_t timeout) {
  return xQueueReceive(uart_rx_queue, byte, timeout);
}