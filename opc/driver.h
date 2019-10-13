#ifndef LEDSCAPE_OPC_DRIVER_H
#define LEDSCAPE_OPC_DRIVER_H

#include <stdint.h>

struct buffer_pixel_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;
} __attribute__((__packed__));

class Driver {
public:
  Driver(int num_strips, int num_pixels_per_strip);
  virtual ~Driver() = default;
 
  int num_strips() const { return num_strips_; }
  int num_pixels_per_strip() const { return num_pixels_per_strip_; }

  virtual void SetPixelData(buffer_pixel_t* rgba_data, int num_pixels) = 0;

private:
  const int num_strips_;
  const int num_pixels_per_strip_;
};

#endif // LEDSCAPE_OPC_DRIVER_H
