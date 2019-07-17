#include "opc/ledscape_driver.h"

#include <string>

#include "opc/server-pru.h"

LedscapeDriver::LedscapeDriver(const server_config_t &server_config)
    : server_config_(server_config) {
  Init();
}

void LedscapeDriver::Init() {
  std::string pru0_filename = build_pruN_program_name(
      server_config_.output_mode_name, server_config_.output_mapping_name, 0);
  std::string pru1_filename = build_pruN_program_name(
      server_config_.output_mode_name, server_config_.output_mapping_name, 1);

  // Init LEDscape
  printf("[main] Starting LEDscape... leds_per_strip %d, pru0_program %s, "
         "pru1_program %s\n",
         server_config_.leds_per_strip, pru0_filename.c_str(),
         pru1_filename.c_str());
  leds_ =
      ledscape_init_with_programs(server_config_.leds_per_strip,
                                  pru0_filename.c_str(), pru1_filename.c_str());
}

void LedscapeDriver::SetPixelData(uint8_t* rgba_data, int num_pixels) {
  ledscape_set_rgba_data(leds_, server_config_.color_channel_order, rgba_data,
                         num_pixels);
  ledscape_draw(leds_);
  //  fprintf(stderr, "Sending rgba data to ledscape %d:%d:%d\n",
  // render_state->backing_data[0].r,
  // render_state->backing_data[0].g,
  // render_state->backing_data[0].b);
}
