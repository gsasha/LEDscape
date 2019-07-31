#ifndef LEDSCAPE_OPC_RENDER_H
#define LEDSCAPE_OPC_RENDER_H

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include "opc/driver.h"
#include "opc/rate-data.h"

class RenderState {
public:
  RenderState(Driver *driver);

  void StartThread();
  void JoinThread();

  void SetStripData(int strip, buffer_pixel_t *pixels, int num_pixels);

private:
  static void *ThreadFunc(void *render_state);
  void Thread();
  void RenderBackingData();

  Driver* driver_;

  const int num_pixels_;

  pthread_t thread_handle_;
  buffer_pixel_t *frame_data_;

  RateData rate_data_;
};

#endif // LEDSCAPE_OPC_RENDER_H
