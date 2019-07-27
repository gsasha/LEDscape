#ifndef LEDSCAPE_OPC_DEMO_H
#define LEDSCAPE_OPC_DEMO_H

void demo_black(int strip_index, uint8_t *strip_buffer, int strip_pixels,
                int frame, struct timeval now);

void demo_identify(int strip_index, uint8_t *strip_buffer, int strip_pixels,
                   int frame, struct timeval now);
#endif // LEDSCAPE_OPC_DEMO_H
