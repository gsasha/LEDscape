#include "opc/ledscape_driver.h"

#include <string>

#include "opc/color.h"
#include "opc/server-pru.h"

LedscapeDriver::LedscapeDriver(const server_config_t &server_config)
    : Driver(server_config.used_strip_count, server_config.leds_per_strip),
      server_config_(server_config), lut_enabled_(server_config.lut_enabled) {
  Init();
}

void LedscapeDriver::Init() {
  if (lut_enabled_) {
    BuildLookupTables();
  }

  std::string pru0_filename = build_pruN_program_name(
      server_config_.output_mode_name, server_config_.output_mapping_name, 0);
  std::string pru1_filename = build_pruN_program_name(
      server_config_.output_mode_name, server_config_.output_mapping_name, 1);

  printf("[main] Starting LEDscape... leds_per_strip %d, pru0_program %s, "
         "pru1_program %s\n",
         server_config_.leds_per_strip, pru0_filename.c_str(),
         pru1_filename.c_str());
  leds_ =
      ledscape_init_with_programs(server_config_.leds_per_strip,
                                  pru0_filename.c_str(), pru1_filename.c_str());
}

void LedscapeDriver::BuildLookupTables() {
  double lum_power = server_config_.lum_power;

  compute_lookup_table(server_config_.white_point.red, lum_power,
                       lut_lookup_red_);
  compute_lookup_table(server_config_.white_point.green, lum_power,
                       lut_lookup_green_);
  compute_lookup_table(server_config_.white_point.blue, lum_power,
                       lut_lookup_blue_);
}

void LedscapeDriver::SetPixelData(buffer_pixel_t* pixels, int num_pixels) {
  // Apply LUT to the data.
  uint8_t *lut_lookup_r = lut_lookup_red_;
  uint8_t *lut_lookup_g = lut_lookup_green_;
  uint8_t *lut_lookup_b = lut_lookup_blue_;
  for (int i = 0; i < num_pixels; i++) {
    buffer_pixel_t *pixel = &pixels[i];
    pixel->r = lut_lookup_r[pixel->r];
    pixel->g = lut_lookup_g[pixel->g];
    pixel->b = lut_lookup_b[pixel->b];
  }
  ledscape_set_rgba_data(leds_, server_config_.color_channel_order,
                         (uint8_t *)pixels, num_pixels);
  ledscape_draw(leds_);
  //  fprintf(stderr, "Sending rgba data to ledscape %d:%d:%d\n",
  // render_state->backing_data[0].r,
  // render_state->backing_data[0].g,
  // render_state->backing_data[0].b);
}
