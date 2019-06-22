#ifndef LEDSCAPE_OPC_FRAME_STATE_H
#define LEDSCAPE_OPC_FRAME_STATE_H

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include "opc/rate-data.h"
#include "opc/server-config.h"

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t unused;
} __attribute__((__packed__)) buffer_pixel_t;

typedef struct {
  int num_strips_used;
  int leds_per_strip;
  int num_leds;

  pthread_mutex_t frame_data_mutex;
  buffer_pixel_t *frame_data;
  buffer_pixel_t *backing_data;

  ledscape_t* leds;

  uint8_t lut_lookup_red[257];
  uint8_t lut_lookup_green[257];
  uint8_t lut_lookup_blue[257];
  bool lut_enabled;
 
  struct rate_data rate_data;
} render_state_t;

void init_render_state(render_state_t *render_state,
                       server_config_t *server_config);

void set_strip_data(render_state_t *render_state, int strip,
                    buffer_pixel_t *strip_data, int strip_num_pixels);

void *render_thread(void *render_state);

#endif // LEDSCAPE_OPC_FRAME_STATE_H
