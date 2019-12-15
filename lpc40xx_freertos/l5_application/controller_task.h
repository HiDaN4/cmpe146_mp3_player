#pragma once
#include <stdbool.h>

typedef enum lcd_menu_screen { MAIN, DETAILS } lcd_menu_screen_e;

void controller_task(void *params);
bool lcd_is_playing(void);