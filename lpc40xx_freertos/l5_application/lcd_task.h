#pragma once

typedef enum LCD_BUTTON { UP, DOWN, PLAY, BACK, VOL_UP, VOL_DOWN } lcd_button_e;

void lcd_menu_task(void *params);