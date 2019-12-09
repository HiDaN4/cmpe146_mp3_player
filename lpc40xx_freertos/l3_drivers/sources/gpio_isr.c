#include <stdint.h>

#include "gpio_isr.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

static const int pin_size = 32;

static function_pointer_t gpio2_callbacks_falling[32];
static function_pointer_t gpio2_callbacks_rising[32];

int gpio2__detect_pin() {
  for (int i = 0; i < pin_size; i++) {
    if (LPC_GPIOINT->IO2IntStatR & (1 << i) || LPC_GPIOINT->IO2IntStatF & (1 << i))
      return i;
  }
  return 0;
}

void gpio2__clear_pin_interrupt(int pin) { LPC_GPIOINT->IO2IntClr |= (1 << pin); }

void gpio_isr__init(void) {
  NVIC_EnableIRQ(GPIO_IRQn);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, &gpio2__interrupt_dispatcher);
}

void gpio2__attach_interrupt(uint8_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    gpio2_callbacks_rising[pin] = callback;
    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
  } else {
    gpio2_callbacks_falling[pin] = callback;
    LPC_GPIOINT->IO2IntEnF |= (1 << pin);
  }
}

void gpio__attach_interrupt(gpio_s gpio, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  gpio2__attach_interrupt(gpio.pin_number, interrupt_type, callback);
}

void gpio2__interrupt_dispatcher(void) {
  const int interrupted_pin = gpio2__detect_pin();
  function_pointer_t attached_user_handler;

  if (LPC_GPIOINT->IO2IntStatR & (1 << interrupted_pin)) {
    attached_user_handler = gpio2_callbacks_rising[interrupted_pin];
  } else {
    attached_user_handler = gpio2_callbacks_falling[interrupted_pin];
  }

  attached_user_handler();
  gpio2__clear_pin_interrupt(interrupted_pin);
}