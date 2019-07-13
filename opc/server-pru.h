#ifndef LEDSCAPE_OPC_SERVER_PRU_H
#define LEDSCAPE_OPC_SERVER_PRU_H

#include <inttypes.h>

#include <string>

std::string build_pruN_program_name(const std::string &output_mode_name,
                                    const std::string &output_mapping_name,
                                    uint8_t pruNum);

#endif // LEDSCAPE_OPC_SERVER_PRU_H
