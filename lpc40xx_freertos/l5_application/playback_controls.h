#pragma once

typedef enum CONTROL_BUTTON { UP, DOWN, PLAY, BACK } control_button_e;

void playback_controls__initialize(void);

void playback_controls_task(void *params);