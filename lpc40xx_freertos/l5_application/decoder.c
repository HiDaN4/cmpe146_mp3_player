#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decoder.h"

#include "gpio.h"
#include "gpio_isr.h"
#include "ssp0_driver.h"

#include "FreeRTOS.h"
#include "queue.h"

/// Set to non-zero to enable debugging, and then you can use MP3__DEBUG_PRINTF()
#define MP3__ENABLE_DEBUGGING 1

#if MP3__ENABLE_DEBUGGING
#define MP3__DEBUG_PRINTF(f_, ...)                                                                                     \
  do {                                                                                                                 \
    fprintf(stderr, "MP3:");                                                                                           \
    fprintf(stderr, (f_), ##__VA_ARGS__);                                                                              \
    fprintf(stderr, "\n");                                                                                             \
  } while (0)
#else
#define MP3__DEBUG_PRINTF(f_, ...) /* NOOP */
#endif

extern xQueueHandle Q_songdata;
const int data_size_bytes = 512;

static gpio_s mp3_dreq;

static gpio_s mp3_xcs;
static gpio_s mp3_xdcs;
static gpio_s mp3_reset;

static enum state_m playing_state;

void vs__write_register(uint8_t address, uint8_t high, uint8_t low);
void process_byte(char byte);
void vs_decoder__initialize(void);

void vs__write_register(uint8_t address, uint8_t high, uint8_t low);
uint16_t vs__read_register(uint8_t register_address);

bool mp3_is_in_reset(void) { return gpio__get(mp3_reset) == false; }

bool mp3_is_ready(void) { return gpio__get(mp3_dreq); }

void mp3_cs(void) { gpio__reset(mp3_xcs); }
void mp3_ds(void) { gpio__set(mp3_xcs); }

void mp3_data_cs(void) { gpio__reset(mp3_xdcs); }
void mp3_data_ds(void) { gpio__set(mp3_xdcs); }

/// Put VS1053 into hardware reset
void mp3__reset(void) { gpio__reset(mp3_reset); }

/*
 * Set VS SCI_VOL register's left and right master volume level in -0.5 dB Steps.
 * Maximum volume = 0x0000.
 * Total silence = 0xFEFE.
 * Data Sheet Section 8.7.11.
 *
 * input values are -1/2dB. ~> 20 will be considered -10dB.
 */
void mp3__set_volume(uint8_t left_channel, uint8_t right_channel) {
  vs__write_register(SCI_VOL, left_channel, right_channel);
}

void mp3_decoder__initialize(void) {
  mp3_dreq = gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_input(mp3_dreq);

  mp3_xcs = gpio__construct_with_function(GPIO__PORT_2, 2, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_output(mp3_xcs);
  mp3_ds();

  mp3_xdcs = gpio__construct_with_function(GPIO__PORT_2, 1, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_output(mp3_xdcs);
  mp3_data_ds();

  mp3_reset = gpio__construct_with_function(GPIO__PORT_2, 4, GPIO__FUNCITON_0_IO_PIN);
  gpio__set_as_output(mp3_reset);
  gpio__set(mp3_reset);

  gpio_s miso = gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCTION_2);
  gpio__set_as_input(miso);

  gpio_s mosi = gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCTION_2);
  gpio__set_as_output(mosi);

  gpio_s scl = gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2);
  gpio__set_as_output(scl);
}

/// Initialize VS1053 chip
void vs_decoder__initialize(void) {
  MP3__DEBUG_PRINTF("Initializing the VS decoder...");
  const uint8_t delay = 100;

  vTaskDelay(delay);
  mp3__reset();
  vTaskDelay(delay);

  // dereset the decoder
  gpio__set(mp3_reset);

  ssp0_driver__init(1);

  vTaskDelay(delay);

  uint16_t status = vs__read_register(SCI_STATUS);

  MP3__DEBUG_PRINTF("Received VS status = 0x%02X", status);

  vs__write_register(SCI_MODE, 0x08, 0x00);

  uint16_t mode = vs__read_register(SCI_MODE);

  MP3__DEBUG_PRINTF("The value in VS mode = 0x%02X %i", mode, mode);

  // increase the internal clock multiplier
  // set the multiplier to 3.0x
  vs__write_register(SCI_CLOCKF, 0x30, 0x00);
  // Internal clock multiplier = 3x
  vTaskDelay(delay);

  // Set to 48Khz on stereo
  vs__write_register(SCI_AUDATA, 0xBB, 0x81);

  uint16_t vs_clock = vs__read_register(SCI_CLOCKF);

  MP3__DEBUG_PRINTF("Received VS clock value = 0x%02X", vs_clock);

  ssp0_driver__set_clock(5);

  mp3__set_volume(40, 40);

  uint16_t vol = vs__read_register(SCI_VOL);

  MP3__DEBUG_PRINTF("The value in VS vol = 0x%02X %i", vol, vol);

  vTaskDelay(delay);
}

void vs__write_register(uint8_t address, uint8_t high, uint8_t low) {
  MP3__DEBUG_PRINTF("VS writing register 0x%02X", address);

  if (mp3_is_in_reset()) {
    MP3__DEBUG_PRINTF("VS is in reset");
    return;
  }

  while (!mp3_is_ready())
    ; // wait

  // TODO: consider mutex
  mp3_cs();
  {
    ssp0_driver__exchange_byte(0x02); // write instruction
    ssp0_driver__exchange_byte(address);
    ssp0_driver__exchange_byte(high);
    ssp0_driver__exchange_byte(low);

    while (!mp3_is_ready())
      ;
  }
  mp3_ds();
  MP3__DEBUG_PRINTF("VS done writing");
}

uint16_t vs__read_register(uint8_t register_address) {
  MP3__DEBUG_PRINTF("VS reading register 0x%02X...", register_address);
  union two_byte_reg result;

  if (mp3_is_in_reset()) {
    MP3__DEBUG_PRINTF("VS is in reset mode...");
    return 0;
  }

  while (mp3_is_ready() == false) {
    MP3__DEBUG_PRINTF("VS is not ready... Waiting...");
    vTaskDelay(1); // repeat until available
  }

  // TODO: consider mutex
  mp3_cs();
  {
    ssp0_driver__exchange_byte(0x03);
    ssp0_driver__exchange_byte(register_address);

    // read the values
    result.byte[1] = ssp0_driver__exchange_byte(0xFF);
    while (mp3_is_ready() == false)
      ;
    result.byte[0] = ssp0_driver__exchange_byte(0xFF);
    while (mp3_is_ready() == false)
      ;
  }
  mp3_ds();

  return result.word;
}

void mp3__enable_data_interrupt(void) {
  // gpio_isr__init();
  // gpio__attach_interrupt(mp3_dreq, GPIO_INTR__RISING_EDGE, &mp3__needs_data_interrupt);
}

void mp3__prepare_for_play(void) {
  vs__write_register(SCI_DECODE_TIME, 0, 0); // Reset decode and bitrate from any prev play back
}

void mp3__print_decode_time(void) {
  uint16_t res = vs__read_register(SCI_DECODE_TIME);
  MP3__DEBUG_PRINTF("Value in SCI_DECODE_TIME = 0x%02X", res);
}

void mp3__print_red_data(uint8_t address) {
  uint16_t res = vs__read_register(address);
  MP3__DEBUG_PRINTF("Value in 0x%02X = 0x%02X", address, res);
}

bool mp3_decoder_needs_data(void) { return gpio__get(mp3_dreq); }

/// Process given byte of data
void process_byte(char byte) {
  mp3_data_cs();
  { ssp0_driver__exchange_byte(byte); }
  mp3_data_ds();
}

void process_bytes(char *bytes, int start, int end) {
  mp3_data_cs();
  {
    for (int i = start; i < end; ++i)
      ssp0_driver__exchange_byte(bytes[i]);
  }
  mp3_data_ds();
}

// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *params) {
  char data[data_size_bytes];
  // initialize data to 0
  memset(data, 0, data_size_bytes);

  mp3_decoder__initialize();
  mp3_ds();
  mp3_data_ds();
  vs_decoder__initialize();
  playing_state = initialized;
  mp3__prepare_for_play();

  mp3__print_red_data(SCI_STATUS);
  mp3__print_red_data(SCI_VOL);

  while (1) {
    xQueueReceive(Q_songdata, &data[0], portMAX_DELAY);
    playing_state = playback;

    for (int i = 0; i < data_size_bytes; ++i) {
      if (!mp3_decoder_needs_data()) {
        vTaskDelay(1);
      }
      while (gpio__get(mp3_dreq) == false)
        ;
      process_byte(data[i]);
    }
    mp3__print_red_data(SCI_DECODE_TIME);
  }
}

/*

// algorithm to find stack size
typedef stack {
  void* start;
  size_t size;
} task_s;

task_s *h = ...;

//
  _______
 |_______| 10,000 start
 |       |
 |       |
 |       |
 |_______| 5,000 stack end here
 |       |
 |       |
 |_______|
//

int *end = h->start - h->size + 1; // -1 since we want to get the one before

while (*end == 0xAAAAAAAA) { // 0xAAAAAAAA is watermark
  ++end;
}
size_t used_bytes = (end - h->start);
size_t free_bytes = (h->size - used_bytes);

*/