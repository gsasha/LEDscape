#include "opc/render.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>

#include "opc/color.h"
#include "opc/server-pru.h"

RenderState::RenderState(const server_config_t &server_config)
    : used_strip_count(server_config.used_strip_count),
      leds_per_strip(server_config.leds_per_strip),
      num_leds(leds_per_strip * used_strip_count),
      color_channel_order(server_config.color_channel_order),
      frame_data_mutex(PTHREAD_MUTEX_INITIALIZER), frame_data(nullptr),
      backing_data(nullptr), lut_enabled(server_config.lut_enabled),
      rate_data(5) {

  if (lut_enabled) {
    BuildLookupTables(server_config);
  }

  InitLedscape(server_config);

  uint32_t led_count = (uint32_t)(leds_per_strip)*LEDSCAPE_NUM_STRIPS;

  frame_data =
      static_cast<buffer_pixel_t *>(malloc(led_count * sizeof(buffer_pixel_t)));
  backing_data =
      static_cast<buffer_pixel_t *>(malloc(led_count * sizeof(buffer_pixel_t)));
}

void RenderState::StartThread() {
  pthread_create(&thread_handle, NULL, &RenderState::ThreadFunc, this);
}

void RenderState::JoinThread() { pthread_join(thread_handle, NULL); }

void RenderState::SetStripData(int strip, buffer_pixel_t *strip_data,
                               int strip_num_leds) {
  strip = strip;
  strip_data = strip_data, strip_num_leds = strip_num_leds;
  pthread_mutex_lock(&frame_data_mutex);
  buffer_pixel_t *frame_strip_data = frame_data + strip * leds_per_strip;
  memcpy(frame_strip_data, strip_data, strip_num_leds * sizeof(buffer_pixel_t));
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
    memcpy(backing_data, frame_data,
           used_strip_count * leds_per_strip * sizeof(buffer_pixel_t));
    pthread_mutex_unlock(&frame_data_mutex);

    RenderBackingData();

    if (rate_data.AddEvent()) {
      printf("[render] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data.GetTotalEvents(), rate_data.GetTotalRatePerSec(),
             rate_data.GetRecentRatePerSec());
    }
  }
}

void RenderState::BuildLookupTables(const server_config_t &server_config) {
  double lum_power = server_config.lum_power;

  compute_lookup_table(server_config.white_point.red, lum_power,
                       lut_lookup_red);
  compute_lookup_table(server_config.white_point.green, lum_power,
                       lut_lookup_green);
  compute_lookup_table(server_config.white_point.blue, lum_power,
                       lut_lookup_blue);
}

void RenderState::InitLedscape(const server_config_t &server_config) {

  std::string pru0_filename = build_pruN_program_name(
      server_config.output_mode_name, server_config.output_mapping_name, 0);
  std::string pru1_filename = build_pruN_program_name(
      server_config.output_mode_name, server_config.output_mapping_name, 1);

  // Init LEDscape
  printf("[main] Starting LEDscape... leds_per_strip %d, pru0_program %s, "
         "pru1_program %s\n",
         server_config.leds_per_strip, pru0_filename.c_str(),
         pru1_filename.c_str());
  leds =
      ledscape_init_with_programs(server_config.leds_per_strip,
                                  pru0_filename.c_str(), pru1_filename.c_str());
}

void RenderState::RenderBackingData() {
  // Apply LUT to the data.
  uint8_t *lut_lookup_r = lut_lookup_red;
  uint8_t *lut_lookup_g = lut_lookup_green;
  uint8_t *lut_lookup_b = lut_lookup_blue;
  for (int i = 0; i < num_leds; i++) {
    buffer_pixel_t *pixel = &backing_data[i];
    pixel->r = lut_lookup_r[pixel->r];
    pixel->g = lut_lookup_g[pixel->g];
    pixel->b = lut_lookup_b[pixel->b];
  }
  // Send data to ledscape.
  ledscape_set_rgba_data(leds, color_channel_order,
                         reinterpret_cast<uint8_t *>(backing_data), num_leds);
  ledscape_draw(leds);
  //  fprintf(stderr, "Sending rgba data to ledscape %d:%d:%d\n",
  // render_state->backing_data[0].r,
  // render_state->backing_data[0].g,
  // render_state->backing_data[0].b);
}

