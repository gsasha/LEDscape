#include "opc/server-pru.h"

#include <stdio.h>

void build_pruN_program_name(const char *output_mode_name,
                             const char *output_mapping_name, uint8_t pruNum,
                             char *out_pru_filename, int filename_len) {
  snprintf(out_pru_filename, filename_len, "pru/bin/%s-%s-pru%d.bin",
           output_mode_name, output_mapping_name, (int)pruNum);
}

