#include "lpc40xx.h"
#include <stdio.h>

#include "clock.h"
#include "gpio.h"
#include "lpc_peripherals.h"
#include "ssp2.h"

void ssp0__initialize(uint32_t max_clock_mhz) {

  // Refer to LPC User manual and setup the register bits correctly
  // a) Power on Peripheral
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  // b) Setup control registers CR0 and CR1

  // Select how many bits to send
  LPC_SSP0->CR0 = (7 << 0);

  // Enable Master
  LPC_SSP0->CR1 = (1 << 1);

  uint8_t divider = 2;

  // set clock in MegaHz instead of KiloHz
  const uint32_t cpu_clock_mhz = clock__get_core_clock_hz() / 1000000UL;

  // Keep scaling down divider until calculated is higher
  while (max_clock_mhz < (cpu_clock_mhz / divider) && divider <= 254) {
    divider += 2;
  }

  LPC_SSP0->CPSR = divider;
}

uint8_t ssp0__exchange_byte(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the status register
  LPC_SSP0->DR = data_out;
  while (LPC_SSP0->SR & (1 << 4)) {
  }

  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}

const int lcd_cs_pin = 15;
static gpio_s lcd_cs_gpio;

static void lcd_cs(void) { gpio__reset(lcd_cs_gpio); }
static void lcd_ds(void) { gpio__set(lcd_cs_gpio); }

void configure_ssp2() {
  gpio_s sck = gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4);
  gpio__set_as_output(sck);

  gpio_s mosi = gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4);
  gpio__set_as_output(mosi);

  gpio_s miso = gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4);
  gpio__set_as_input(miso);

  lcd_cs_gpio = gpio__construct_with_function(GPIO__PORT_0, lcd_cs_pin, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_output(lcd_cs_gpio);

  lcd_ds();
}

void lcd__initialize(void) {
  uint32_t max_clock_speed_khz = 125;
  ssp2__initialize(max_clock_speed_khz);

  configure_ssp2();
  return;

  // SCK0 (0,15)
  gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2);

  // set the MOSI0 (P0_18)
  gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCTION_2);

  // set the MISO0 (P0_17)
  gpio_s miso = gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCTION_2);

  // SetAsOutput
  // CS0(0,16)
  lcd_cs_gpio = gpio__construct_as_output(GPIO__PORT_0, lcd_cs_pin);
  gpio__set_function(lcd_cs_gpio, GPIO__FUNCITON_0_IO_PIN);
}

void lcd_display_string(char *data) {
  fprintf(stderr, "Printing chars\n");
  lcd_cs();

  for (int x = 0; data[x] != '\0'; x++) // Send chars until we hit the end of the string
    ssp2__exchange_byte(data[x]);

  lcd_ds();
  fprintf(stderr, "Done printing chars\n");
}

void lcd_clear(void) {
  lcd_cs();
  ssp2__exchange_byte('|');
  ssp2__exchange_byte('-');
  lcd_ds();
}