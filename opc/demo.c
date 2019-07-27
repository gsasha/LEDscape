#include "opc/demo.h"

void demo_black(int strip_index, uint8_t *strip_buffer, int strip_pixels,
                int frame, struct timeval now) {
  memset(strip_buffer, strip_pixels * 3, 0);
}

void demo_identify(int strip_index, uint8_t *strip_buffer, int strip_pixels,
                int frame, struct timeval now) {
  memset(strip_buffer, strip_pixels * 3, 0);
  int i;
  int strip_bits = strip_index;
  for (i = 0; i < sizeof(strip_index) * 8 && i < strip_pixels; i++) {
    int strip_bit = strip_bits & 1;
    strip_bits >>= 1;
    strip_buffer[i * 3 + 0] = strip_buffer[i * 3 + 1] =
        strip_buffer[i * 3 + 2] = strip_bit ? 255 : 0;
  }
}

