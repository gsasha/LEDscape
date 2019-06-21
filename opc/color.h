#ifndef LEDSCAPE_OPC_COLOR_H
#define LEDSCAPE_OPC_COLOR_H

#include <stdint.h>

// Convert hue, saturation and brightness ( HSB/HSV ) to RGB
// The dim_curve is used only on brightness/value and on saturation
// (inverted). This looks the most natural.
void HSBtoRGB(int32_t hue, int32_t sat, int32_t val, uint8_t out[]);

// Interpolate to a given white point for 257 entries.
void compute_lookup_table(float white_point, double lum_power,
                          uint32_t *lookup);

#endif // LEDSCAPE_OPC_COLOR_H
