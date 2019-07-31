#ifndef LEDSCAPE_OPC_FLTK_DRIVER_H
#define LEDSCAPE_OPC_FLTK_DRIVER_H

#include "opc/driver.h"

class FltkDriver : public Driver {
public:
  FltkDriver(int argc, char *argv[], int num_strips, int num_pixels_per_strip);
  virtual void SetPixelData(buffer_pixel_t *pixels, int num_pixels) override;

  void Run();

private:
  class PixelRenderer;

  PixelRenderer *renderer_;
};

#endif // LEDSCAPE_OPC_FLTK_DRIVER_H
