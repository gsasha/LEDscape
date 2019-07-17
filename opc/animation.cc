#include "opc/animation.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ledscape/ledscape.h"

void StripAnimation::Init(int num_pixels) {
  pixels = (buffer_pixel_t *)malloc(num_pixels * sizeof(buffer_pixel_t));
  memset((void *)pixels, 0, num_pixels * sizeof(buffer_pixel_t));
  gettimeofday(&enable_time, NULL);
}

void *Animation::ThreadFunc(void *animation_ptr) {
  auto *animation_state = static_cast<Animation *>(animation_ptr);
  animation_state->Thread();
  return nullptr;
}

Animation::Animation(const server_config_t &server_config, Driver *driver)
    : server_config(server_config), render_state_(server_config, driver),
      rate_data_(60) {}

void Animation::Init() {
  for (int i = 0; i < LEDSCAPE_NUM_STRIPS; i++) {
    strips_[i].Init(server_config.leds_per_strip);
  }
}

void Animation::StartThread() {
  pthread_create(&thread_handle, NULL, &Animation::ThreadFunc, this);
  render_state_.StartThread();
}

void Animation::JoinThread() { pthread_join(thread_handle, NULL); }

void Animation::Thread() {
  int num_strips = server_config.used_strip_count;
  int strip_num_pixels = server_config.leds_per_strip;

  struct timeval now;
  gettimeofday(&now, NULL);

  RateScheduler rate_scheduler(60);

  for (;;) {
    rate_scheduler.WaitFrame();

    // TODO(gsasha): add thread cancellation?
    for (int strip_index = 0; strip_index < num_strips; strip_index++) {
      auto *strip = &strips_[strip_index];
      if (!strip->enabled) {
        if (timercmp(&strip->enable_time, &now, >)) {
          // Strip will start executing on next round.
          strip->enabled = true;
        }
        continue;
      }
      // Render this strip.
      if (strip_index % 2 == 1) {
        memset(strip->pixels, 0xff, strip_num_pixels * sizeof(buffer_pixel_t));
      }
      render_state_.SetStripData(strip_index, strip->pixels, strip_num_pixels);
    }

    if (rate_data_.AddEvent()) {
      printf("[animation] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_.GetTotalEvents(),
             rate_data_.GetTotalRatePerSec(),
             rate_data_.GetRecentRatePerSec());
    }
  }
}

