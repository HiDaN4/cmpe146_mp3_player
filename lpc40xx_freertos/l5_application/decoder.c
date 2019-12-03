#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decoder.h"

#include "gpio.h"
#include "ssp2_driver.h"

#include "FreeRTOS.h"
#include "queue.h"

extern xQueueHandle Q_songdata;
const int data_size_bytes = 512;

static gpio_s mp3_dreq;
static gpio_s mp3_xcs;
static gpio_s mp3_xdcs;
static gpio_s mp3_reset;

bool mp3_is_in_reset(void) { return gpio__get(mp3_reset) == false; }

bool mp3_is_ready(void) { return gpio__get(mp3_dreq); }

void mp3_cs(void) { gpio__reset(mp3_xcs); }
void mp3_ds(void) { gpio__set(mp3_xcs); }

void mp3_xd_cs(void) { gpio__reset(mp3_xdcs); }
void mp3_xd_ds(void) { gpio__set(mp3_xdcs); }


/// Put VS1053 into hardware reset
void mp3__reset(void) { gpio__reset(mp3_reset); }


uint16_t vs__read_register(uint8_t register_address) {
    union two_byte_reg result;

    if (mp3_is_in_reset()) {
        return 0;
    }

    while(mp3_is_ready() == false) {
        ; // repeat until available
    }

    mp3_cs();
    {
        ssp2_driver__exchange_byte(0x03);
        ssp2_driver__exchange_byte(register_address);

        // read the values
        result.byte[1] = ssp2_driver__exchange_byte(0xFF);
        while(mp3_is_ready() == false) ;
        result.byte[0] = ssp2_driver__exchange_byte(0xFF);
        while(mp3_is_ready() == false) ;
    }
    mp3_ds();

    return result.word;
}


/// Initialize VS1053 chip
void vs_decoder__initialize(void) {
    const uint8_t delay = 100;

    vTaskDelay(delay);
    gpio__reset(mp3_reset);
    vTaskDelay(delay);

    // dereset the decoder
    gpio__set(mp3_reset);

    ssp2_driver__init(2);

    int mode = vs__read_register()
}


void mp3_decoder__initialize(void) {
    /*
    pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
  pinMode(MP3_RESET, OUTPUT);
    */
   mp3_dreq = gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCITON_0_IO_PIN);
   gpio__set_as_input(mp3_dreq);

   mp3_xcs = gpio__construct_with_function(GPIO__PORT_2, 2, GPIO__FUNCITON_0_IO_PIN);
   gpio__set_as_output(mp3_xcs);

   mp3_xdcs = gpio__construct_with_function(GPIO__PORT_2, 1, GPIO__FUNCITON_0_IO_PIN);
   gpio__set_as_output(mp3_xdcs);

   mp3_reset = gpio__construct_with_function(GPIO__PORT_2, 4, GPIO__FUNCITON_0_IO_PIN);
   gpio__set_as_output(mp3_reset);

   gpio_s miso = gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCITON_0_IO_PIN);
   gpio__set_as_input(miso);

   gpio_s mosi = gpio__construct_with_function(GPIO__PORT_1, 1, );
   gpio__set_as_output(mosi);
   
   gpio_s scl = gpio__construct_with_function(GPIO__PORT_1, 0, );
   gpio__set_as_output(scl);
}

/// To be done
bool mp3_decoder_needs_data(void) { return true; }

/// Process given byte of data. For now, it just prints it
void process_byte(char byte) { 
    putchar(byte);
    ssp2_driver__exchange_byte(byte);
}

// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *params) {
  char data[data_size_bytes];
  // initialize data to 0
  memset(data, 0, data_size_bytes);

  mp3_decoder__initialize();
  mp3_ds();
  mp3_xd_ds();

  while (1) {
    xQueueReceive(Q_songdata, &data[0], portMAX_DELAY);

    for (int i = 0; i < data_size_bytes; ++i) {
      if (!mp3_decoder_needs_data()) {
        vTaskDelay(1);
      }
      process_byte(data[i]);
    }
    printf("\nFinished processing data...\n");
  }
}