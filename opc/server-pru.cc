#include "opc/server-pru.h"

#include <stdio.h>

#include <sstream>

std::string build_pruN_program_name(const std::string &output_mode_name,
                                    const std::string &output_mapping_name,
                                    uint8_t pruNum) {
  std::ostringstream os;
  os << "pru/bin/" << output_mode_name << "-" << output_mapping_name << "-pru"
     << pruNum << ".bin";
  return os.str();
}

