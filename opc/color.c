#include "opc/color.h"

#include <math.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

void HSBtoRGB(int32_t hue, int32_t sat, int32_t val, uint8_t out[]) {
  /* convert hue, saturation and brightness ( HSB/HSV ) to RGB
           The dim_curve is used only on brightness/value and on saturation
     (inverted). This looks the most natural.
  */

  int r = 0, g = 0, b = 0;
  int base;

  if (sat == 0) { // Achromatic color (gray). Hue doesn't mind.
    r = g = b = val;
  } else {
    base = ((255 - sat) * val) >> 8;

    switch ((hue % 360) / 60) {
    case 0:
      r = val;
      g = (((val - base) * hue) / 60) + base;
      b = base;
      break;

    case 1:
      r = (((val - base) * (60 - (hue % 60))) / 60) + base;
      g = val;
      b = base;
      break;

    case 2:
      r = base;
      g = val;
      b = (((val - base) * (hue % 60)) / 60) + base;
      break;

    case 3:
      r = base;
      g = (((val - base) * (60 - (hue % 60))) / 60) + base;
      b = val;
      break;

    case 4:
      r = (((val - base) * (hue % 60)) / 60) + base;
      g = base;
      b = val;
      break;

    case 5:
      r = val;
      g = base;
      b = (((val - base) * (60 - (hue % 60))) / 60) + base;
      break;
    }

    out[0] = (uint8_t)r;
    out[1] = (uint8_t)g;
    out[2] = (uint8_t)b;
  }
}

void compute_lookup_table(float white_point, double lum_power,
                          uint32_t *lookup) {

  for (uint16_t i = 0; i < 257; i++) {
    double normalI = i * white_point / 256.;

    double output = pow(normalI, lum_power);
    int64_t longOutput = (int64_t)((output * 0xFFFF) + 0.5);
    int32_t clampedOutput = (int32_t)max(0, min(0xFFFF, longOutput));

    lookup[i] = (uint32_t)clampedOutput;
  }
}

