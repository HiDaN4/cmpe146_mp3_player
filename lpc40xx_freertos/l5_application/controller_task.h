#pragma once
#include <stdbool.h>

typedef enum lcd_menu_screen { MAIN, DETAILS } lcd_menu_screen_e;

typedef enum mp3_reader_action { PLAY, PAUSE } mp3_reader_action_e;

void controller_task(void *params);
bool controller_is_playing(void);