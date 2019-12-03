#pragma once

void mp3_decoder__initialize(void);

union two_byte_reg {
/**
 * \brief whole word value
 *
 * allows access and handeling of whole uint16_t (aka word) value
 */
  uint16_t word;

/**
 * \brief individual bytes
 *
 * allows access and handeling of individual uint8_t (aka bytes),
 * either MSB or LSB byte of word
 */
  uint8_t  byte[2];
} ;




/**
 * \brief A macro of the SCI MODE register's address (R/W)
 *
 * SCI_MODE is a Read/Write register used to control the operation of VS1053b and defaults to 0x0800
 * (SM_SDINEW set).
 *
 */
#define SCI_MODE        0x00