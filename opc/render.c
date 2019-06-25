#define __USE_BSD
#include "opc/render.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>

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
  (*render_state) = (render_state_t){
      .num_strips_used = server_config->used_strip_count,
      .leds_per_strip = server_config->leds_per_strip,
      .num_leds = render_state->leds_per_strip * render_state->num_strips_used,
      .color_channel_order = server_config->color_channel_order,

      .frame_data_mutex = PTHREAD_MUTEX_INITIALIZER,
      .frame_data = NULL,
      .backing_data = NULL,

      .leds = NULL,

      .lut_lookup_red = {0},
      .lut_lookup_green = {0},
      .lut_lookup_blue = {0},
      .lut_enabled = false,
  };

  render_state->lut_enabled = server_config->lut_enabled;
  if (render_state->lut_enabled) {
    build_lookup_tables(server_config, render_state);
  }

  init_ledscape(server_config, render_state);

  uint32_t led_count =
      (uint32_t)(render_state->leds_per_strip) * LEDSCAPE_NUM_STRIPS;

  render_state->frame_data = malloc(led_count * sizeof(buffer_pixel_t));
  render_state->backing_data = malloc(led_count * sizeof(buffer_pixel_t));

  init_rate_data(&render_state->rate_data, 5);
}

void set_strip_data(render_state_t *render_state, int strip,
                    buffer_pixel_t *strip_data, int strip_num_leds) {
render_state = render_state; strip=strip; strip_data = strip_data, strip_num_leds = strip_num_leds;
  pthread_mutex_lock(&render_state->frame_data_mutex);
  buffer_pixel_t *frame_strip_data =
    render_state->frame_data + strip * render_state->leds_per_strip;
  memcpy(frame_strip_data, strip_data,
         strip_num_leds * sizeof(buffer_pixel_t));
  pthread_mutex_unlock(&render_state->frame_data_mutex);
}

void render_backing_data(render_state_t* render_state) {
  // Apply LUT to the data.
  uint8_t *lut_lookup_r = render_state->lut_lookup_red;
  uint8_t *lut_lookup_g = render_state->lut_lookup_green;
  uint8_t *lut_lookup_b = render_state->lut_lookup_blue;
  for (int i = 0; i < render_state->num_leds; i++) {
    buffer_pixel_t *pixel = &render_state->backing_data[i];
    pixel->r = lut_lookup_r[pixel->r];
    pixel->g = lut_lookup_g[pixel->g];
    pixel->b = lut_lookup_b[pixel->b];
  }
  // Send data to ledscape.
  ledscape_set_rgba_data(render_state->leds, render_state->color_channel_order,
                         (uint8_t *)render_state->backing_data,
                         render_state->num_leds);
  ledscape_draw(render_state->leds);
//  fprintf(stderr, "Sending rgba data to ledscape %d:%d:%d\n",
// render_state->backing_data[0].r,
// render_state->backing_data[0].g,
// render_state->backing_data[0].b);
}

void render_thread_run(render_state_t *render_state) {
  struct rate_scheduler_t rate_scheduler;
  init_rate_scheduler(&rate_scheduler, 60);

  fprintf(stderr, "Starting render thread\n");
  for (int frame = 0;; frame++) {
    // fprintf(stderr, "Frame %d\n", frame);
    rate_scheduler_wait_frame(&rate_scheduler);

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
