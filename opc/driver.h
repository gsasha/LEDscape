#ifndef LEDSCAPE_OPC_DRIVER_H
#define LEDSCAPE_OPC_DRIVER_H

#include <stdint.h>

class Driver {
public:
  virtual ~Driver() = default;

  virtual void SetPixelData(uint8_t* rgba_data, int num_pixels) = 0;
};

#endif // LEDSCAPE_OPC_DRIVER_H
