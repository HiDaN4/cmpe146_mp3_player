#pragma once
#include <stdint.h>

extern const uint8_t lcd_rows;
extern const uint8_t lcd_columns;

void lcd__initialize(void);

void lcd_display_string(char *data);
void lcd_display_string_starting_at(char *data, uint8_t line);
void lcd_display_string_at(char *data, uint8_t line, uint8_t column);

void lcd_display_character(char character);
void lcd_display_character_at(char character, uint8_t line, uint8_t column);
void lcd_set_cursor(uint8_t line, uint8_t column);

void lcd_remove_character_at(uint8_t line, uint8_t column);
void lcd_clear_line(uint8_t line);
void lcd_clear(void);