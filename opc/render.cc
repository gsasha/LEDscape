#include "opc/render.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>

RenderState::RenderState(Driver *driver)
    : driver_(driver),
      num_pixels_(driver->num_strips() * driver->num_pixels_per_strip()),
      frame_data_(new buffer_pixel_t[num_pixels_]), rate_data_(10) {}

void RenderState::StartThread() {
  pthread_create(&thread_handle_, NULL, &RenderState::ThreadFunc, this);
}

void RenderState::JoinThread() { pthread_join(thread_handle_, NULL); }

void RenderState::SetStripData(int strip, buffer_pixel_t *pixels,
                               int num_pixels) {
  // There is a race between SetStipData and rendering to the driver, but it's
  // plain data, so there is no danger in the race so there is no need to lock.
  buffer_pixel_t *frame_strip_data =
      frame_data_ + strip * driver_->num_pixels_per_strip();
  memcpy(frame_strip_data, pixels, num_pixels * sizeof(buffer_pixel_t));
}

void *RenderState::ThreadFunc(void *render_state) {
  static_cast<RenderState *>(render_state)->Thread();
  return nullptr;
}

void RenderState::Thread() {
  fprintf(stderr, "Starting render thread\n");

  RateScheduler rate_scheduler(60);

  for (int frame = 0;; frame++) {
    rate_scheduler.WaitFrame();

    driver_->SetPixelData(frame_data_, num_pixels_);

    if (rate_data_.AddEvent()) {
      printf("[render] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_.GetTotalEvents(), rate_data_.GetTotalRatePerSec(),
             rate_data_.GetRecentRatePerSec());
    }
  }
}

