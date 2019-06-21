#ifndef LEDSCAPE_OPC_SERVER_PRU_H
#define LEDSCAPE_OPC_SERVER_PRU_H

#include <inttypes.h>

void build_pruN_program_name(const char *output_mode_name,
                             const char *output_mapping_name, uint8_t pruNum,
                             char *out_pru_filename, int filename_len);

#endif // LEDSCAPE_OPC_SERVER_PRU_H
