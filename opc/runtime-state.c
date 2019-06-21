#include "opc/runtime-state.h"

#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "opc/color.h"
#include "opc/server-pru.h"

void init_ledscape(runtime_state_t* runtime_state) {
  server_config_t *server_config = &runtime_state->server_config;
  char pru0_filename[4096], pru1_filename[4096];

  build_pruN_program_name(server_config->output_mode_name,
                          server_config->output_mapping_name, 0, pru0_filename,
                          sizeof(pru0_filename));

  build_pruN_program_name(server_config->output_mode_name,
                          server_config->output_mapping_name, 1, pru1_filename,
                          sizeof(pru1_filename));

  // Init LEDscape
  printf("[main] Starting LEDscape... leds_per_strip %d, pru0_program %s, "
         "pru1_program %s",
         server_config->leds_per_strip, pru0_filename, pru1_filename);
  runtime_state->leds = ledscape_init_with_programs(
      server_config->leds_per_strip, pru0_filename, pru1_filename);
}

void setup_runtime_state(runtime_state_t *runtime_state) {
  // This is running before any threads are started so no need to lock.
  printf("Initializing runtime state...");

  // Setup tables
  init_render_state(&runtime_state->server_config, &runtime_state->render_state);
  init_ledscape(runtime_state);
}

