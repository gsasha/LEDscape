#ifndef LEDSCAPE_OPC_SERVER_CONFIG_H
#define LEDSCAPE_OPC_SERVER_CONFIG_H

#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>

#include "ledscape/ledscape.h"

typedef enum {
  DEMO_MODE_NONE = 0,
  DEMO_MODE_FADE = 1,
  DEMO_MODE_IDENTIFY = 2,
  DEMO_MODE_BLACK = 3,
  DEMO_MODE_POWER = 4
} demo_mode_t;

typedef struct {
  char output_mode_name[512];
  char output_mapping_name[512];

  demo_mode_t demo_mode;
  demo_mode_t demo_mode_per_strip[LEDSCAPE_NUM_STRIPS];

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
} server_config_t;

int read_config_file(const char *config_filename, server_config_t *out_config);
int write_config_file(const char *config_filename, server_config_t *config);

// Config Methods
int validate_server_config(server_config_t *input_config,
                           char *result_json_buffer,
                           size_t result_json_buffer_size);

int server_config_from_json(const char *json, size_t json_size,
                            server_config_t *output_config);

void server_config_to_json(char *dest_string, size_t dest_string_size,
                           server_config_t *input_config);

const char *demo_mode_to_string(demo_mode_t mode);

demo_mode_t demo_mode_from_string(const char *str);

void print_server_config(FILE* file, server_config_t* server_config);

#endif // LEDSCAPE_OPC_SERVER_CONFIG_H

