#include "opc/animation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void StripAnimation::Init(int num_pixels) {
  pixels = (buffer_pixel_t *)malloc(num_pixels * sizeof(buffer_pixel_t));
  memset((void *)pixels, 0, num_pixels * sizeof(buffer_pixel_t));
  gettimeofday(&enable_time, NULL);
}

Animation::Animation(Driver *driver)
    : driver_(driver), render_state_(driver), rate_data_(60) {
  Init();
}

void Animation::Init() {
  strips_.resize(driver_->num_strips());

  for (int i = 0; i < driver_->num_strips(); i++) {
    strips_[i].Init(driver_->num_pixels_per_strip());
    if (i < 24) {
      strips_[i].effect = new StarsEffect(
          strips_[i].pixels, driver_->num_pixels_per_strip(), i * 5 + 15);
    } else if (i < 25) {
      strips_[i].effect =
          new BreatheEffect(strips_[i].pixels, driver_->num_pixels_per_strip());
    }
    else if (i < 30) {
      strips_[i].effect = new WalkEffect(
          strips_[i].pixels, driver_->num_pixels_per_strip(), rand() % 100);
    } else if ( i < 40) {
      int offset = rand() % 4 + i;
      double step = (rand() % 1000) / 100000.0;
      strips_[i].effect = new ColorFadeEffect(
          strips_[i].pixels, driver_->num_pixels_per_strip(), offset, step);
    } else  {
      strips_[i].effect = new MatrixEffect(
          strips_[i].pixels, driver_->num_pixels_per_strip(), 5, true);
    }
  }
}

void Animation::StartThread() {
  pthread_create(&thread_handle, NULL, &Animation::ThreadFunc, this);
  render_state_.StartThread();
}

void Animation::JoinThread() { pthread_join(thread_handle, NULL); }

void Animation::Thread() {
  RateScheduler rate_scheduler(60);

  for (;;) {
    rate_scheduler.WaitFrame();

    struct timeval now;
    gettimeofday(&now, NULL);

    // TODO(gsasha): add thread cancellation?
    for (int strip_index = 0; strip_index < driver_->num_strips();
         strip_index++) {
      auto *strip = &strips_[strip_index];
      if (!strip->enabled) {
        if (timercmp(&now, &strip->enable_time, >)) {
          // Strip will start executing on next round.
          strip->enabled = true;
        }
        continue;
      }
      // Render this strip.
      if (strip->effect != nullptr) {
        strip->effect->RenderFrame();
      } else {
        if (strip_index % 2 == 1) {
          memset(strip->pixels, 0xaa,
                 driver_->num_pixels_per_strip() * sizeof(buffer_pixel_t));
        }
      }
      render_state_.SetStripData(strip_index, strip->pixels,
                                 driver_->num_pixels_per_strip());
    }

    if (rate_data_.AddEvent()) {
      printf("[animation] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_.GetTotalEvents(),
             rate_data_.GetTotalRatePerSec(),
             rate_data_.GetRecentRatePerSec());
    }
  }
}

void *Animation::ThreadFunc(void *animation_ptr) {
  auto *animation_state = static_cast<Animation *>(animation_ptr);
  animation_state->Thread();
  return nullptr;
}


