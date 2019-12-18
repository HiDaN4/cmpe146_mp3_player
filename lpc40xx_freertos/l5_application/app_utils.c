#include "app_utils.h"

int convert_volume_value(int new_volume) {
  static const int orig_max = 0x00;
  static const int orig_min = 0xFE;
  static const int new_max = 100;
  static const int new_min = 0;

  const int old_range = (orig_max - orig_min);
  const int new_range = (new_max - new_min);

  return (new_volume - new_min) * (orig_max - orig_min) / (new_max - new_min) + orig_min;
}