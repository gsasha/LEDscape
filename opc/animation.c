#include "opc/animation.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

void init_strip_animation_state(
    strip_animation_state_t *strip_animation_state,
    int num_pixels) {
  *strip_animation_state = (strip_animation_state_t){
      .pixels = malloc(num_pixels * sizeof(buffer_pixel_t)),
      .animation_type = 0,
      .animation_state = NULL,
      .enabled = false,
  };
  gettimeofday(&strip_animation_state->enable_time, NULL);
  memset(strip_animation_state->pixels, 0, num_pixels * sizeof(buffer_pixel_t));
}

void init_animation_state(animation_state_t *animation_state,
                          server_config_t *server_config,
                          render_state_t* render_state) {
  animation_state->server_config = server_config;
  animation_state->render_state = render_state;

  init_rate_data(&animation_state->rate_data, 60);
}

void* animation_thread(void* animation_state_ptr) {
  animation_state_t *animation_state = (animation_state_t *)animation_state_ptr;

  int num_strips = animation_state->server_config->used_strip_count;
  struct timeval now;
  gettimeofday(&now, NULL);

  struct rate_scheduler_t rate_scheduler;
  init_rate_scheduler(&rate_scheduler, 60);

  for (;;) {
    rate_scheduler_wait_frame(&rate_scheduler);

    // TODO(gsasha): add thread cancellation?
    for (int strip_index = 0; strip_index < num_strips; strip_index++) {
      strip_animation_state_t *strip = &animation_state->strip[strip_index];
      if (!strip->enabled) {
        if (timercmp(&strip->enable_time, &now, >)) {
           // Strip will start executing on next round.
           strip->enabled = true;
        }
        continue;
      }
      // Render this strip.
    }

    struct rate_data_t* rate_data = &animation_state->rate_data;
    if (rate_data_add_event(rate_data)) {
      printf("[animation] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_get_total_events(rate_data),
             rate_data_get_total_rate_per_sec(rate_data),
             rate_data_get_recent_rate_per_sec(rate_data));
    }
  }
}

void start_animation_thread(animation_state_t* animation_state) {
  pthread_create(&animation_state->thread_handle, NULL, animation_thread,
                 animation_state);
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

