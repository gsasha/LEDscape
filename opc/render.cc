#include "opc/render.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>

//#include "opc/color.h"
//#include "opc/server-pru.h"

RenderState::RenderState(Driver *driver)
    : driver_(driver),
      num_pixels_(driver->num_strips() * driver->num_pixels_per_strip()),
      frame_data_mutex(PTHREAD_MUTEX_INITIALIZER), frame_data(nullptr),
      backing_data(nullptr),
      // lut_enabled(server_config.lut_enabled),
      rate_data(5) {

  /*
    if (lut_enabled) {
      BuildLookupTables(server_config);
    }
  */

  frame_data = static_cast<buffer_pixel_t *>(
      malloc(num_pixels_ * sizeof(buffer_pixel_t)));
  backing_data = static_cast<buffer_pixel_t *>(
      malloc(num_pixels_ * sizeof(buffer_pixel_t)));
}

void RenderState::StartThread() {
  pthread_create(&thread_handle, NULL, &RenderState::ThreadFunc, this);
}

void RenderState::JoinThread() { pthread_join(thread_handle, NULL); }

void RenderState::SetStripData(int strip, buffer_pixel_t *pixels,
                               int num_pixels) {
  pthread_mutex_lock(&frame_data_mutex);
  buffer_pixel_t *frame_strip_data =
      frame_data + strip * driver_->num_pixels_per_strip();
  memcpy(frame_strip_data, pixels, num_pixels * sizeof(buffer_pixel_t));
  pthread_mutex_unlock(&frame_data_mutex);
}

void *RenderState::ThreadFunc(void *render_state) {
  static_cast<RenderState *>(render_state)->Thread();
  return nullptr;
}

void RenderState::Thread() {
  RateScheduler rate_scheduler(60);

  fprintf(stderr, "Starting render thread\n");
  for (int frame = 0;; frame++) {
    // fprintf(stderr, "Frame %d\n", frame);
    rate_scheduler.WaitFrame();

    pthread_mutex_lock(&frame_data_mutex);
    memcpy(backing_data, frame_data, num_pixels_ * sizeof(buffer_pixel_t));
    pthread_mutex_unlock(&frame_data_mutex);

    RenderBackingData();

    if (rate_data.AddEvent()) {
      printf("[render] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data.GetTotalEvents(), rate_data.GetTotalRatePerSec(),
             rate_data.GetRecentRatePerSec());
    }
  }
}

void RenderState::RenderBackingData() {
driver_->SetPixelData(backing_data, num_pixels_);
}

