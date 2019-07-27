#ifndef LEDSCAPE_OPC_FLTK_DRIVER_H
#define LEDSCAPE_OPC_FLTK_DRIVER_H

#include <pthread.h>

#include "opc/driver.h"

class FltkDriver : public Driver {
public:
  FltkDriver(int argc, char *argv[], int num_strips, int num_pixels_per_strip);
  virtual void SetPixelData(uint8_t *rgba_data, int num_pixels) override;

private:
  static void* ThreadFunc(void* this_ptr);

  class PixelRenderer;

  const int num_strips_;
  const int num_pixels_per_strip_;
  PixelRenderer* renderer_;
  pthread_t thread_handle_;
};

#endif // LEDSCAPE_OPC_FLTK_DRIVER_H
