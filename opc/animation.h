#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

#include "sys/time.h"

#include "ledscape/ledscape.h"
#include "opc/server-config.h"
#include "opc/render.h"

typedef struct {
  int animation_type;
  void* animation_state;
  struct timespec enable_time;

} strip_animation_state_t;

typedef struct {
  server_config_t* server_config;
  render_state_t* render_state;
  int num_used_strips;
  strip_animation_state_t strip_animation_state[LEDSCAPE_NUM_STRIPS];
} animation_state_t;

void init_animation_state(animation_state_t *animation_state,
                          server_config_t *server_config,
                          render_state_t* render_state);

void start_animation_thread(animation_state_t* animation_state);
void stop_animation_thread(animation_state_t* animation_state);

void set_animation_mode_all(animation_state_t *animation_state,
                            char *animation_mode);

void set_animation_mode_strip(animation_state_t *animation_state, int strip,
                              char *animation_mode);

void animation_disable_all(animation_state_t *animation_state, double seconds);

void animation_disable_strip(animation_state_t *animation_state, double seconds,
                             int strip);

#endif // LEDSCAPE_OPC_ANIMATION_H