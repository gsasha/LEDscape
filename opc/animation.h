#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

#include <pthread.h>
#include <sys/time.h>

#include "ledscape/ledscape.h"
#include "opc/rate-data.h"
#include "opc/render.h"
#include "opc/server-config.h"

typedef struct {
  buffer_pixel_t* pixels;  
  int animation_type;
  void* animation_state;
  bool enabled;
  struct timeval enable_time;
} strip_animation_state_t;

typedef struct {
  server_config_t* server_config;
  render_state_t* render_state;

  pthread_t thread_handle;

  strip_animation_state_t strip[LEDSCAPE_NUM_STRIPS];

  struct rate_data_t rate_data;
} animation_state_t;

void init_animation_state(animation_state_t *animation_state,
                          server_config_t *server_config,
                          render_state_t* render_state);

void start_animation_thread(animation_state_t* animation_state);
void join_animation_thread(animation_state_t* animation_state);

void set_animation_mode_all(animation_state_t *animation_state,
                            char *animation_mode);

void set_animation_mode_strip(animation_state_t *animation_state, int strip,
                              char *animation_mode);

void animation_disable_all(animation_state_t *animation_state, double seconds);

void animation_disable_strip(animation_state_t *animation_state, double seconds,
                             int strip);

#endif // LEDSCAPE_OPC_ANIMATION_H
