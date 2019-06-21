#ifndef LEDSCAPE_OPC_RUNTIME_STATE_H
#define LEDSCAPE_OPC_RUNTIME_STATE_H

#include "opc/server-config.h"

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} __attribute__((__packed__)) buffer_pixel_t;

// Pixel Delta
typedef struct {
  int8_t r;
  int8_t g;
  int8_t b;

  int8_t last_effect_frame_r;
  int8_t last_effect_frame_g;
  int8_t last_effect_frame_b;
} __attribute__((__packed__)) pixel_delta_t;

typedef struct {
  buffer_pixel_t *previous_frame_data;
  buffer_pixel_t *current_frame_data;
  buffer_pixel_t *next_frame_data;

  pixel_delta_t *frame_dithering_overflow;

  uint8_t has_prev_frame;
  uint8_t has_current_frame;
  uint8_t has_next_frame;

  uint32_t frame_size;
  uint32_t leds_per_strip;

  volatile uint32_t frame_counter;

  struct timeval previous_frame_tv;
  struct timeval current_frame_tv;
  struct timeval next_frame_tv;

  struct timeval prev_current_delta_tv;

  ledscape_t *leds;

  char pru0_program_filename[4096];
  char pru1_program_filename[4096];

  uint32_t red_lookup[257];
  uint32_t green_lookup[257];
  uint32_t blue_lookup[257];

  struct timeval last_remote_data_tv;

  pthread_mutex_t mutex;
} runtime_state_t;

void setup_runtime_state(server_config_t *sever_config,
                         runtime_state_t *runtime_state);

#endif // LEDSCAPE_OPC_RUNTIME_STATE_H
