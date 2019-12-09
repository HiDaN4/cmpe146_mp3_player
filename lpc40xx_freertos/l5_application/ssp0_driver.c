#include "ssp0_driver.h"
#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

void ssp0_driver__init(uint32_t max_clock_mhz) {
  // Power on Peripheral
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  LPC_SSP0->CR0 = 7;
  LPC_SSP0->CR1 = (1 << 1);

  ssp0_driver__set_clock(max_clock_mhz);
}

void ssp0_driver__set_clock(uint32_t max_clock_mhz) {
  // Setup prescalar register to be <= max_clock_mhz
  const uint32_t cpu_clock_khz = clock__get_core_clock_hz() / 1000UL;
  const uint8_t reg_max_value = 254;
  const uint32_t max_clock_khz = max_clock_mhz * 1000;
  uint8_t reg_even_value_to_set = 2;

  while (max_clock_khz < (cpu_clock_khz / reg_even_value_to_set) && reg_even_value_to_set <= reg_max_value) {
    reg_even_value_to_set += 2;
  }

  fprintf(stderr, "Clock speed: %lu\n", cpu_clock_khz);
  fprintf(stderr, "Calculated divider value: %u\n", reg_even_value_to_set);

  LPC_SSP0->CPSR = reg_even_value_to_set;
}

uint8_t ssp0_driver__exchange_byte(uint8_t data_out) {
  // DR (data register) is used for write and read
  LPC_SSP0->DR = data_out;

  while (LPC_SSP0->SR & (1 << 4)) {
    // wait while busy
  }

  // read the value and return it
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}