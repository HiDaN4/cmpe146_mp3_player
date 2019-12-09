#pragma once
#include <stdint.h>

void ssp0_driver__init(uint32_t max_clock_mhz);

void ssp0_driver__set_clock(uint32_t max_clock_mhz);

uint8_t ssp0_driver__exchange_byte(uint8_t data_out);