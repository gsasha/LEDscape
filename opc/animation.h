#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

#include <pthread.h>
#include <sys/time.h>

#include <vector>

#include "opc/driver.h"
#include "opc/effect.h"
#include "opc/rate-data.h"
#include "opc/render.h"

class StripAnimation {
public:
  void Init(int num_pixels);

  buffer_pixel_t *pixels = nullptr;
  Effect *effect = nullptr;
  bool enabled = false;
  struct timeval enable_time = {0, 0};
};

class Animation {
public:
  Animation(Driver* driver);

  void Init();
  void StartThread();
  void JoinThread();

private:
  void Thread();
  static void* ThreadFunc(void* animation_ptr);

  Driver* driver_;
  RenderState render_state_;

  pthread_t thread_handle;

  std::vector<StripAnimation> strips_;

  RateData rate_data_;
};

#endif // LEDSCAPE_OPC_ANIMATION_H
