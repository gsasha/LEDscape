#define __USE_BSD
#include "opc/render-state.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

#include "opc/color.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

void build_lookup_tables(server_config_t *server_config,
                         render_state_t *render_state) {
  double lum_power = server_config->lum_power;

  compute_lookup_table(server_config->white_point.red, lum_power,
                       render_state->lut_lookup_red);
  compute_lookup_table(server_config->white_point.green, lum_power,
                       render_state->lut_lookup_green);
  compute_lookup_table(server_config->white_point.blue, lum_power,
                       render_state->lut_lookup_blue);
}

void init_render_state(server_config_t *server_config,
                      render_state_t *render_state) {
  build_lookup_tables(server_config, render_state);

  uint32_t led_count =
      (uint32_t)(server_config->leds_per_strip) * LEDSCAPE_NUM_STRIPS;

  if (render_state->frame_size != led_count) {
    fprintf(stderr, "Allocating buffers for %d pixels (%ju bytes)\n", led_count,
            (uintmax_t)(led_count * 3 /*channels*/ * 4 /*buffers*/ *
                        sizeof(uint16_t)));

    if (render_state->previous_frame_data != NULL) {
      free(render_state->previous_frame_data);
      free(render_state->current_frame_data);
      free(render_state->next_frame_data);
      free(render_state->frame_dithering_overflow);
    }

    render_state->frame_size = led_count;
    render_state->previous_frame_data =
        malloc(led_count * sizeof(buffer_pixel_t));
    render_state->current_frame_data =
        malloc(led_count * sizeof(buffer_pixel_t));
    render_state->next_frame_data =
        malloc(led_count * sizeof(buffer_pixel_t));
    render_state->frame_dithering_overflow =
        malloc(led_count * sizeof(pixel_delta_t));
    render_state->has_next_frame = false;
    printf("frame_size1=%u\n", render_state->frame_size);

    // Init timestamps
    gettimeofday(&render_state->previous_frame_tv, NULL);
    gettimeofday(&render_state->current_frame_tv, NULL);
    gettimeofday(&render_state->next_frame_tv, NULL);
  }
}

/**
 * Set the next frame of data to the given 8-bit RGB buffer after rotating the
 * buffers.
 */
void set_next_frame_data(render_state_t *render_state, uint8_t *frame_data,
                         uint32_t data_size, uint8_t is_remote) {
  pthread_mutex_lock(&render_state->mutex);

  rotate_frames(render_state, false);

  // Prevent buffer overruns
  data_size = min(data_size, render_state->frame_size * 3);

  // Copy in new data
  memcpy(render_state->next_frame_data, frame_data, data_size);

  // Zero out any pixels not set by the new frame
  memset((uint8_t *)render_state->next_frame_data + data_size, 0,
         (render_state->frame_size * 3 - data_size));

  // Update the timestamp & count
  gettimeofday(&render_state->next_frame_tv, NULL);

  // Update remote data timestamp if applicable
  if (is_remote) {
    gettimeofday(&render_state->last_remote_data_tv, NULL);
  }

  render_state->has_next_frame = true;

  pthread_mutex_unlock(&render_state->mutex);
}

/**
 * Rotate the buffers, dropping the previous frame and loading in the new one
 *
 * if (has_current) previous <-> current;
 * if (has_next) current <-> next
 *
 */
void rotate_frames(render_state_t* render_state, uint8_t lock_frame_data) {
  if (lock_frame_data) {
    pthread_mutex_lock(&render_state->mutex);
  }

  buffer_pixel_t *temp = NULL;

  render_state->has_prev_frame = false;

  if (render_state->has_current_frame) {
    render_state->previous_frame_tv = render_state->current_frame_tv;

    temp = render_state->previous_frame_data;
    render_state->previous_frame_data = render_state->current_frame_data;
    render_state->current_frame_data = temp;

    render_state->has_prev_frame = true;
    render_state->has_current_frame = false;
  }

  if (render_state->has_next_frame) {
    render_state->current_frame_tv = render_state->next_frame_tv;

    temp = render_state->current_frame_data;
    render_state->current_frame_data = render_state->next_frame_data;
    render_state->next_frame_data = temp;

    render_state->has_current_frame = true;
    render_state->has_next_frame = false;
  }

  // Update the delta time stamp
  if (render_state->has_current_frame && render_state->has_prev_frame) {
    timersub(&render_state->current_frame_tv,
             &render_state->previous_frame_tv,
             &render_state->prev_current_delta_tv);
  }

  if (lock_frame_data) {
    pthread_mutex_unlock(&render_state->mutex);
  }
}

void timeval_add(struct timeval *dst, struct timeval *added) {
  dst->tv_usec += added->tv_usec;
  if (dst->tv_usec > 1000000) {
    dst->tv_usec -= 1000000;
    dst->tv_sec++;
  }
  dst->tv_sec += added->tv_sec;
}

// return true if a<b.
int timeval_lt(struct timeval *a, struct timeval *b) {
  if (a->tv_sec < b->tv_sec) {
    return true;
  }
  if (a->tv_sec > b->tv_sec) {
    return false;
  }
  return a->tv_usec < b->tv_usec;
}

// return microseconds of b-a;
int timeval_until(struct timeval *a, struct timeval *b) {
  return (b->tv_sec - a->tv_sec) * 1000000 + b->tv_usec - a->tv_usec;
}

void set_strip_data(render_state_t *render_state, int strip,
                    buffer_pixel_t *strip_data, int strip_num_pixels) {
  pthread_mutex_lock(&render_state->frame_data_mutex);
  buffer_pixel_t *frame_strip_data =
      render_state->frame_data + strip * render_state->pixels_per_strip;
  memcpy(frame_strip_data, strip_data,
         strip_num_pixels * sizeof(buffer_pixel_t));
  pthread_mutex_unlock(&render_state->frame_data_mutex);
}

void render_backing_data(render_state_t* render_state) {
  // Apply LUT to the data.
  // Send data to ledscape.
}

void render_thread(render_state_t *render_state) {
  struct timeval frame_tv;
  gettimeofday(&frame_tv, NULL);
  struct timeval step_frame_tv;
  step_frame_tv.tv_sec = 0;
  step_frame_tv.tv_usec = 1000000 / 60;

  for (;;) {
    timeval_add(frame_tv, step_frame_tv);

    struct timeval current_time_tv;
    gettimeofday(&current_time_tv, NULL);

    if (timeval_lt(current_time_tv, frame_tv)) {
      usleep(timeval_until(current_time_tv, frame_tv));
    }

    pthread_mutex_lock(&render_state->frame_data_mutex);
    memcpy(render_state->backing_data, render_state->frame_data,
           render_state->frame_size);
    pthread_mutex_unlock(&render_state->frame_data_mutex);

    render_backing_data(render_state);
  }
}

