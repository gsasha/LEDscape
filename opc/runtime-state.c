#include "opc/runtime-state.h"

#include <stdio.h>

void init_runtime_state(runtime_state_t *runtime_state) {
  // This is running before any threads are started so no need to lock.
  printf("Initializing runtime state...");

  runtime_state->animation_state = create_animation_state(
      &runtime_state->server_config);
}

