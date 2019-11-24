#include <stdint.h>

void ssp2_driver__init(uint32_t max_cloch_mhz);

uint8_t ssp2_driver__exchange_byte(uint8_t data_out);