#include "opc/animation.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ledscape/ledscape.h"
#include "opc/effect.h"
#include "opc/rate-data.h"

class StripAnimationState {
public:
  StripAnimationState()
      : pixels(nullptr), effect(nullptr), enabled(false), enable_time({0, 0}) {}
  void Init(int num_pixels) {
    pixels = (buffer_pixel_t *)malloc(num_pixels * sizeof(buffer_pixel_t));
    memset((void *)pixels, 0, num_pixels * sizeof(buffer_pixel_t));
    gettimeofday(&enable_time, NULL);
  }
  buffer_pixel_t *pixels;
  Effect *effect;
  bool enabled;
  struct timeval enable_time;
};

class AnimationState {
public:
  AnimationState(server_config_t *server_config, render_state_t *render_state)
      : server_config(server_config), render_state(render_state) {}

  void Init() {
    init_rate_data(&rate_data, 60);
    for (int i = 0; i < LEDSCAPE_NUM_STRIPS; i++) {
      strips_[i].Init(server_config->leds_per_strip);
    }
  }
  void StartThread();
  void JoinThread();

  void Thread();

private:
  server_config_t *server_config;
  render_state_t *render_state;

  pthread_t thread_handle;

  StripAnimationState strips_[LEDSCAPE_NUM_STRIPS];

  struct rate_data_t rate_data;
};

void *animation_thread(void *animation_state_ptr) {
  auto *animation_state =
      static_cast<AnimationState *>(animation_state_ptr);
  animation_state->Thread();
  return nullptr;
}

void AnimationState::StartThread() {
  pthread_create(&thread_handle, NULL, &animation_thread, this);
}

void AnimationState::JoinThread() { pthread_join(thread_handle, NULL); }

void AnimationState::Thread() {
  int num_strips = server_config->used_strip_count;
  int strip_num_pixels = server_config->leds_per_strip;

  struct timeval now;
  gettimeofday(&now, NULL);

  struct rate_scheduler_t rate_scheduler;
  init_rate_scheduler(&rate_scheduler, 60);

  for (;;) {
    rate_scheduler_wait_frame(&rate_scheduler);

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
      set_strip_data(render_state, strip_index, strip->pixels,
                     strip_num_pixels);
    }

    if (rate_data_add_event(&rate_data)) {
      printf("[animation] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_get_total_events(&rate_data),
             rate_data_get_total_rate_per_sec(&rate_data),
             rate_data_get_recent_rate_per_sec(&rate_data));
    }
  }
}

struct animation_state_t {
  AnimationState *impl;
};

animation_state_t *create_animation_state(server_config_t *server_config,
                                          render_state_t *render_state) {
  animation_state_t *animation_state =
      (animation_state_t *)malloc(sizeof(animation_state_t));
  memset(animation_state, 0, sizeof(animation_state_t));

  animation_state->impl = new AnimationState(server_config, render_state);
  animation_state->impl->Init();

  return animation_state;
}

void start_animation_thread(animation_state_t *animation_state) {
  animation_state->impl->StartThread();
}


void join_animation_thread(animation_state_t *animation_state) {
  animation_state->impl->JoinThread();
}

void set_animation_mode_all(animation_state_t *animation_state,
                            char *animation_mode) {
  animation_state = animation_state;
  animation_mode = animation_mode;
  // TODO(gsasha): implement.
}

void set_animation_mode_strip(animation_state_t *animation_state, int strip,
                              char *animation_mode) {
  animation_state = animation_state;
  strip = strip;
  animation_mode = animation_mode;
  // TODO(gsasha): implement.
}

void animation_disable_all(animation_state_t *animation_state, double seconds) {
  animation_state = animation_state;
  seconds = seconds;
  // TODO(gsasha): implement.
}

void animation_disable_strip(animation_state_t *animation_state, double seconds,
                             int strip) {
  animation_state = animation_state;
  seconds = seconds;
  strip = strip;
  // TODO(gsasha): implement.
}

