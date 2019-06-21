#ifndef LEDSCAPE_OPC_FRAME_STATE_H
#define LEDSCAPE_OPC_FRAME_STATE_H

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

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

  volatile uint32_t frame_counter;

  struct timeval previous_frame_tv;
  struct timeval current_frame_tv;
  struct timeval next_frame_tv;

  struct timeval prev_current_delta_tv;

  struct timeval last_remote_data_tv;

  uint32_t lut_lookup_red[257];
  uint32_t lut_lookup_green[257];
  uint32_t lut_lookup_blue[257];

  pthread_mutex_t mutex;
} render_state_t;

void init_render_state(server_config_t *server_config,
                      render_state_t *render_state);

void set_next_frame_data(render_state_t *render_state, uint8_t *frame_data,
                         uint32_t data_size, uint8_t is_remote);
void rotate_frames(render_state_t *render_state, uint8_t lock_frame_data);

#endif // LEDSCAPE_OPC_FRAME_STATE_H
