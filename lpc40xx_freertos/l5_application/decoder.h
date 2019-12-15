#pragma once

typedef enum play_state {
  uninitialized,
  initialized,
  ready,
  playback,
  paused_playback,
  finished_playback
} play_state_e;

void mp3_decoder__initialize(void);

// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *params);

union two_byte_reg {
  /**
   * \brief whole word value
   *
   * allows access of whole uint16_t (aka word) value
   */
  uint16_t word;

  /**
   * \brief individual bytes
   *
   * allows access and handeling of individual uint8_t bytes,
   * MSB or LSB byte of word
   */
  uint8_t byte[2];
};

// Page 37

/**
 * \brief SCI MODE register's address (R/W)
 *
 * SCI_MODE is a Read/Write register used to control the operation of VS1053b and defaults to 0x0800
 * (SM_SDINEW set).
 *
 */
#define SCI_MODE 0x00

#define SCI_STATUS 0x01

/// Clock freq + multiplier
#define SCI_CLOCKF 0x03

/// Volume control
#define SCI_VOL 0x0B

/// Decode time in seconds
#define SCI_DECODE_TIME 0x04

#define SCI_AUDATA 0x05