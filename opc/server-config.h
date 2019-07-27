#ifndef LEDSCAPE_OPC_SERVER_CONFIG_H
#define LEDSCAPE_OPC_SERVER_CONFIG_H

#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>

#include <string>
#include <vector>

#include "ledscape/ledscape.h"

typedef enum {
  DEMO_MODE_NONE = 0,
  DEMO_MODE_FADE = 1,
  DEMO_MODE_IDENTIFY = 2,
  DEMO_MODE_BLACK = 3,
  DEMO_MODE_POWER = 4
} demo_mode_t;

struct server_config_t {
  std::string output_mode_name;
  std::string output_mapping_name;

  demo_mode_t demo_mode;
  std::vector<demo_mode_t> demo_mode_per_strip;

  uint16_t tcp_port;
  uint16_t udp_port;
  uint16_t e131_port;

  uint32_t leds_per_strip;
  uint32_t used_strip_count;

  color_channel_order_t color_channel_order;

  uint8_t interpolation_enabled;
  uint8_t dithering_enabled;
  uint8_t lut_enabled;

  struct {
    float red;
    float green;
    float blue;
  } white_point;

  float lum_power;
};

void init_server_config(server_config_t* config);

int read_config_file(const char *config_filename, server_config_t *out_config);
int write_config_file(const char *config_filename, server_config_t *config);

// Config Methods
int validate_server_config(server_config_t *input_config,
                           std::string *diagnostic_str);

int server_config_from_json(const char *json, size_t json_size,
                            server_config_t *output_config);

void server_config_to_json(char *dest_string, size_t dest_string_size,
                           server_config_t *input_config);

const char *demo_mode_to_string(demo_mode_t mode);

demo_mode_t demo_mode_from_string(const char *str);

void print_server_config(FILE* file, server_config_t* server_config);

#endif // LEDSCAPE_OPC_SERVER_CONFIG_H

