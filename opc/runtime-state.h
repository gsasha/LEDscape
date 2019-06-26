#ifndef LEDSCAPE_OPC_RUNTIME_STATE_H
#define LEDSCAPE_OPC_RUNTIME_STATE_H

#include <stdint.h>

#include "opc/animation.h"
#include "opc/render.h"
#include "opc/server-config.h"

typedef struct {
  server_config_t server_config;
  render_state_t render_state;
  animation_state_t animation_state;
} runtime_state_t;

void init_runtime_state(runtime_state_t *runtime_state);

#endif // LEDSCAPE_OPC_RUNTIME_STATE_H
