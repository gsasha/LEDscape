#define __USE_BSD
#include "opc/render-state.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "opc/color.h"
#include "opc/server-pru.h"

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

void init_ledscape(server_config_t *server_config,
                   render_state_t *render_state) {
  char pru0_filename[4096], pru1_filename[4096];

  build_pruN_program_name(server_config->output_mode_name,
                          server_config->output_mapping_name, 0, pru0_filename,
                          sizeof(pru0_filename));

  build_pruN_program_name(server_config->output_mode_name,
                          server_config->output_mapping_name, 1, pru1_filename,
                          sizeof(pru1_filename));

  // Init LEDscape
  printf("[main] Starting LEDscape... leds_per_strip %d, pru0_program %s, "
         "pru1_program %s\n",
         server_config->leds_per_strip, pru0_filename, pru1_filename);
  render_state->leds = ledscape_init_with_programs(
      server_config->leds_per_strip, pru0_filename, pru1_filename);
}

void init_render_state(render_state_t *render_state,
                       server_config_t *server_config) {
  render_state->num_strips_used = server_config->used_strip_count;
  render_state->leds_per_strip = server_config->leds_per_strip;

  render_state->lut_enabled = server_config->lut_enabled;
  if (render_state->lut_enabled) {
    build_lookup_tables(server_config, render_state);
  }

  init_ledscape(server_config, render_state);

  uint32_t led_count =
      (uint32_t)(render_state->leds_per_strip) * LEDSCAPE_NUM_STRIPS;

  render_state->frame_data = malloc(led_count * sizeof(buffer_pixel_t));
  render_state->backing_data = malloc(led_count * sizeof(buffer_pixel_t));

  rate_data_init(&render_state->rate_data, 5);
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
int timeval_microseconds_until(struct timeval *a, struct timeval *b) {
  return (b->tv_sec - a->tv_sec) * 1000000 + b->tv_usec - a->tv_usec;
}

void set_strip_data(render_state_t *render_state, int strip,
                    buffer_pixel_t *strip_data, int strip_num_pixels) {
  pthread_mutex_lock(&render_state->frame_data_mutex);
  buffer_pixel_t *frame_strip_data =
      render_state->frame_data + strip * render_state->leds_per_strip;
  memcpy(frame_strip_data, strip_data,
         strip_num_pixels * sizeof(buffer_pixel_t));
  pthread_mutex_unlock(&render_state->frame_data_mutex);
}

void render_backing_data(render_state_t* render_state) {
  // Apply LUT to the data.
  uint8_t *lut_lookup_r = render_state->lut_lookup_red;
  uint8_t *lut_lookup_g = render_state->lut_lookup_green;
  uint8_t *lut_lookup_b = render_state->lut_lookup_blue;
  for (int i = 0; i < render_state->num_leds; i++) {
    buffer_pixel_t* pixel = &render_state->backing_data[i];
    pixel->r = lut_lookup_r[pixel->r];
    pixel->g = lut_lookup_g[pixel->g];
    pixel->b = lut_lookup_b[pixel->b];
  }
  // Send data to ledscape.
  // TODO(gsasha):
  render_state = render_state;
}

void render_thread_run(render_state_t *render_state) {
  struct timeval frame_tv;
  gettimeofday(&frame_tv, NULL);
  struct timeval step_frame_tv;
  step_frame_tv.tv_sec = 0;
  step_frame_tv.tv_usec = 1000000 / 60;

  for (;;) {
    timeval_add(&frame_tv, &step_frame_tv);

    struct timeval current_time_tv;
    gettimeofday(&current_time_tv, NULL);

    if (timeval_lt(&current_time_tv, &frame_tv)) {
      usleep(timeval_microseconds_until(&current_time_tv, &frame_tv));
    }

    pthread_mutex_lock(&render_state->frame_data_mutex);
    memcpy(render_state->backing_data, render_state->frame_data,
           render_state->num_strips_used * render_state->leds_per_strip *
               sizeof(buffer_pixel_t));
    pthread_mutex_unlock(&render_state->frame_data_mutex);

    render_backing_data(render_state);

    if (rate_data_add_event(&render_state->rate_data)) {
      printf("[render] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_get_total_events(&render_state->rate_data),
             rate_data_get_total_rate_per_sec(&render_state->rate_data),
             rate_data_get_recent_rate_per_sec(&render_state->rate_data));
    }
  }
}

void* render_thread(void* render_state) {
  render_thread_run((render_state_t*)render_state);
  return NULL;
}
