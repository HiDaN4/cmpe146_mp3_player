#pragma once

typedef enum CONTROL_BUTTON { UP, DOWN, PLAY, BACK, VOL_UP, VOL_DOWN } control_button_e;

void playback_controls_task(void *params);