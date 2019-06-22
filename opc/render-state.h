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
  uint8_t unused;
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
  int num_strips_used;
  int pixels_per_strip;

  pthread_mutex_t frame_data_mutex;
  pthread_cond_t frame_data_cond;
  buffer_pixel_t *frame_data;
  buffer_pixel_t *backing_data;

  uint32_t lut_lookup_red[257];
  uint32_t lut_lookup_green[257];
  uint32_t lut_lookup_blue[257];
} render_state_t;

void init_render_state(server_config_t *server_config,
                      render_state_t *render_state);

void set_next_frame_data(render_state_t *render_state, uint8_t *frame_data,
                         uint32_t data_size, uint8_t is_remote);
void rotate_frames(render_state_t *render_state, uint8_t lock_frame_data);

void set_strip_data(render_state_t *render_state, int strip,
                    buffer_pixel_t *strip_data, int strip_num_pixels);
void render_thread(render_state_t *render_state);
#endif // LEDSCAPE_OPC_FRAME_STATE_H
