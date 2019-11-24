#include "i2c_slave_init.h"
#include "lpc40xx.h"

void i2c2__slave_init(uint8_t slave_address_to_respond_to) {
  LPC_I2C_TypeDef *lpc_i2c = LPC_I2C2;

  // enable slave and master function
  lpc_i2c->CONSET = 0x44;

  // Set I2C slave address
  lpc_i2c->ADR2 = (slave_address_to_respond_to << 0);
  lpc_i2c->ADR1 = lpc_i2c->ADR0 = lpc_i2c->ADR3 = 0;
}