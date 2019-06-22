#include "opc/runtime-state.h"

#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "opc/color.h"
#include "opc/server-pru.h"

void setup_runtime_state(runtime_state_t *runtime_state) {
  // This is running before any threads are started so no need to lock.
  printf("Initializing runtime state...");

  // Setup tables
  init_render_state(&runtime_state->server_config, &runtime_state->render_state);
  init_ledscape(runtime_state);
}

