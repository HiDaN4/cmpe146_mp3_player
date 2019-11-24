#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "ssp2.h"
#include <stdio.h>

void ssp2_driver__init(uint32_t max_clock_mhz) {
  // Power on Peripheral
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP2);

  LPC_SSP2->CR0 = 7;
  LPC_SSP2->CR1 = (1 << 1);

  // Setup prescalar register to be <= max_clock_mhz
  const uint32_t cpu_clock_khz = clock__get_core_clock_hz() / 1000UL;
  const uint8_t reg_max_value = 254;
  const uint32_t max_clock_khz = max_clock_mhz * 1024;
  uint8_t reg_even_value_to_set = 2;

  while (max_clock_khz < (cpu_clock_khz / reg_even_value_to_set) && reg_even_value_to_set <= reg_max_value) {
    reg_even_value_to_set += 2;
  }

  LPC_SSP2->CPSR = reg_even_value_to_set;
}

uint8_t ssp2_driver__exchange_byte(uint8_t data_out) {
  // DR (data register) is used for write and read
  LPC_SSP2->DR = data_out;

  while (LPC_SSP2->SR & (1 << 4)) {
    // wait while busy
  }

  // read the value and return it
  return (uint8_t)(LPC_SSP2->DR & 0xFF);
}