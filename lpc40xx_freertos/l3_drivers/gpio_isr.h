#pragma once
#include "gpio.h"
#include <stdint.h>

typedef enum { GPIO_INTR__FALLING_EDGE, GPIO_INTR__RISING_EDGE } gpio_interrupt_e;

// Function pointer type (demonstrated later in the code sample)
typedef void (*function_pointer_t)(void);

void gpio_isr__init(void);

// Allow the user to attach their callbacks
void gpio2__attach_interrupt(uint8_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback);
void gpio__attach_interrupt(gpio_s gpio, gpio_interrupt_e interrupt_type, function_pointer_t callback);

void gpio2__interrupt_dispatcher(void);