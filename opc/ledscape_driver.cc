#include "opc/ledscape_driver.h"

#include <iostream>
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

  buffer_ = new uint8_t[server_config_.leds_per_strip *
                        server_config_.used_strip_count * 4];
  buffer_strip_starts_ = new uint8_t *[server_config_.used_strip_count];
  for (int i = 0; i < server_config_.used_strip_count; i++) {
    buffer_strip_starts_[i] = buffer_ + i * server_config_.leds_per_strip * 4;
  }

  printf("LEDscape init done\n");
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
  for (int strip = 0; strip < server_config_.used_strip_count; strip++) {
    buffer_pixel_t * __restrict strip_pixels =
        pixels + server_config_.leds_per_strip * strip;
    uint8_t* __restrict buffer_pos = buffer_strip_starts_[strip];
    // 4 pixels pack into 3 words.
    // The driver endian-transforms every 4 bytes.
    for (int i = 0; i < server_config_.leds_per_strip; i++, buffer_pos += 4) {
        buffer_pos[3] = lut_lookup_green_[strip_pixels[i + 0].g];
        buffer_pos[2] = lut_lookup_red_[strip_pixels[i + 0].r];
        buffer_pos[1] = lut_lookup_blue_[strip_pixels[i + 0].b];
        buffer_pos[0] = lut_lookup_blue_[strip_pixels[i + 0].w];
    }
/*
    for (int i = 0; i < server_config_.leds_per_strip;
         i += 4, buffer_pos += 12) {
      // TODO(gsasha): here maybe apply LUT and also maybe add RGBW.

      if (lut_enabled_) {
        buffer_pos[3] = lut_lookup_green_[strip_pixels[i + 0].g];
        buffer_pos[2] = lut_lookup_red_[strip_pixels[i + 0].r];
        buffer_pos[1] = lut_lookup_blue_[strip_pixels[i + 0].b];

        buffer_pos[0] = lut_lookup_green_[strip_pixels[i + 1].g];
        buffer_pos[7] = lut_lookup_red_[strip_pixels[i + 1].r];
        buffer_pos[6] = lut_lookup_blue_[strip_pixels[i + 1].b];

        buffer_pos[5] = lut_lookup_green_[strip_pixels[i + 2].g];
        buffer_pos[4] = lut_lookup_red_[strip_pixels[i + 2].r];
        buffer_pos[11] = lut_lookup_blue_[strip_pixels[i + 2].b];

        buffer_pos[10] = lut_lookup_green_[strip_pixels[i + 3].g];
        buffer_pos[9] = lut_lookup_red_[strip_pixels[i + 3].r];
        buffer_pos[8] = lut_lookup_blue_[strip_pixels[i + 3].b];
      } else {
        buffer_pos[3] = strip_pixels[i + 0].g;
        buffer_pos[2] = strip_pixels[i + 0].r;
        buffer_pos[1] = strip_pixels[i + 0].b;

        buffer_pos[0] = strip_pixels[i + 1].g;
        buffer_pos[7] = strip_pixels[i + 1].r;
        buffer_pos[6] = strip_pixels[i + 1].b;

        buffer_pos[5] = strip_pixels[i + 2].g;
        buffer_pos[4] = strip_pixels[i + 2].r;
        buffer_pos[11] = strip_pixels[i + 2].b;

        buffer_pos[10] = strip_pixels[i + 3].g;
        buffer_pos[9] = strip_pixels[i + 3].r;
        buffer_pos[8] = strip_pixels[i + 3].b;
      }
    }
*/
  }
  ledscape_set_raw_data(leds_, buffer_, num_pixels);
  ledscape_draw(leds_);
}
