#include "opc/server-config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

#include "frozen.h"
#include "ledscape/util.h"
#include "opc/server-error.h"
#include "opc/server-pru.h"

static const int MAX_CONFIG_FILE_LENGTH_BYTES = 1024 * 1024 * 10;

void init_server_config(server_config_t *config) {
  *config = (server_config_t){/*.output_mode_name =*/"ws281x",
                              /*.output_mapping_name =*/"original-ledscape",

                              .tcp_port = 7890,
                              .udp_port = 7890,

                              .e131_port = 5568,

                              .leds_per_strip = 176,
                              .used_strip_count = LEDSCAPE_NUM_STRIPS,
                              .color_channel_order = COLOR_ORDER_BRG,

                              .lut_enabled = true,

                              .white_point = {.9, 1, 1},
                              .lum_power = 2};
}

int read_config_file(const char *config_filename, server_config_t *out_config) {
  // Map the file for reading
  int fd = open(config_filename, O_RDONLY);
  if (fd < 0) {
    return opc_server_set_error(
        OPC_SERVER_ERR_FILE_READ_FAILED,
        "Failed to open config file %s for reading: %s\n", config_filename,
        strerror(errno));
  }

  off_t file_end_offset = lseek(fd, 0, SEEK_END);

  if (file_end_offset < 0) {
    return opc_server_set_error(OPC_SERVER_ERR_SEEK_FAILED,
                                "Failed to seek to end of %s.\n",
                                config_filename);
  }

  if (file_end_offset > MAX_CONFIG_FILE_LENGTH_BYTES) {
    return opc_server_set_error(
        OPC_SERVER_ERR_FILE_TOO_LARGE,
        "Failed to open config file %s: file is larger than 10MB.\n",
        config_filename);
  }

  size_t file_length = (size_t)file_end_offset;

  void *data = mmap(0, file_length, PROT_READ, MAP_PRIVATE, fd, 0);

  // Read the config
  // TODO: Handle character encoding?
  char *str_data = static_cast<char *>(malloc(file_length + 1));
  memcpy(str_data, data, file_length);
  str_data[file_length] = 0;
  server_config_from_json(str_data, strlen(str_data), out_config);
  free(str_data);

  // Unmap the data
  munmap(data, file_length);

  return close(fd);
}

int write_config_file(const char *config_filename, server_config_t *config) {
  FILE *fd = fopen(config_filename, "w");
  if (fd == NULL) {
    return opc_server_set_error(
        OPC_SERVER_ERR_FILE_WRITE_FAILED,
        "Failed to open config file %s for reading: %s\n", config_filename,
        strerror(errno));
  }

  char json_buffer[4096] = {0};
  server_config_to_json(json_buffer, sizeof(json_buffer), config);
  fputs(json_buffer, fd);

  return fclose(fd);
}

int server_config_from_json(const char *json, size_t json_size,
                            server_config_t *output_config) {
  char *token_value_str;
  int token_value_int;
  float token_value_float;

  if (json_size < 2) {
    // No JSON data
    return opc_server_set_error(OPC_SERVER_ERR_NO_JSON, NULL);
  }

  // Search for parameter "bar" and print it's value
  if (json_scanf(json, json_size, "outputMode:%Q", &token_value_str) > 0) {
    output_config->output_mode_name = token_value_str;
    printf("JSON outputMode %s\n", output_config->output_mode_name.c_str());
    free(token_value_str);
  }

  if (json_scanf(json, json_size, "outputMapping:%Q", &token_value_str) > 0) {
    output_config->output_mapping_name = token_value_str;
    printf("JSON outputMapping %s\n",
           output_config->output_mapping_name.c_str());
    free(token_value_str);
  }

  if (json_scanf(json, json_size, "ledsPerStrip:%d", &token_value_int) > 0) {
    output_config->leds_per_strip = token_value_int;
    printf("JSON ledsPerStrip %d\n", token_value_int);
  }

  if (json_scanf(json, json_size, "usedStripCount:%d", &token_value_int) > 0) {
    output_config->used_strip_count = token_value_int;
    printf("JSON usedStripCount %d\n", token_value_int);
  }

  if (json_scanf(json, json_size, "colorChannelOrder:%Q", &token_value_str) >
      0) {
    output_config->color_channel_order =
        color_channel_order_from_string(token_value_str);
    printf("JSON colorChannelOrder %s\n", token_value_str);
    free(token_value_str);
  }

  if (json_scanf(json, json_size, "opcTcpPort:%d", &token_value_int) > 0) {
    output_config->tcp_port = token_value_int;
    printf("JSON opcTcpPort %d\n", token_value_int);
  }

  if (json_scanf(json, json_size, "opcUdpPort:%d", &token_value_int) > 0) {
    output_config->udp_port = token_value_int;
    printf("JSON opcUdpPort %d\n", token_value_int);
  }

  if (json_scanf(json, json_size, "enableLookupTable:%B", &token_value_int) >
      0) {
    output_config->lut_enabled = token_value_int;
    printf("JSON enableLookupTable %d\n", token_value_int);
  }

  if (json_scanf(json, json_size, "lumCurvePower:%f", &token_value_float) > 0) {
    output_config->lum_power = token_value_float;
    printf("JSON lumCurvePower %f\n", token_value_float);
  }

  if (json_scanf(json, json_size, "whitePoint.red:%f", &token_value_float) >
      0) {
    output_config->white_point.red = token_value_float;
    printf("JSON whitePoint.red %f\n", token_value_float);
  }

  if (json_scanf(json, json_size, "whitePoint.green:%f", &token_value_float) >
      0) {
    output_config->white_point.green = token_value_float;
    printf("JSON whitePoint.green %f\n", token_value_float);
  }

  if (json_scanf(json, json_size, "whitePoint.blue:%f", &token_value_float) >
      0) {
    output_config->white_point.blue = token_value_float;
    printf("JSON whitePoint.blue %f\n", token_value_float);
  }

  return 0;
}

void server_config_to_json(char *dest_string, size_t dest_string_size,
                           server_config_t *input_config) {
  // Build config JSON
  snprintf(dest_string, dest_string_size,

           "{\n"
           "\t"
           "\"outputMode\": \"%s\","
           "\n"
           "\t"
           "\"outputMapping\": \"%s\","
           "\n"

           "\t"
           "\"ledsPerStrip\": %d,"
           "\n"
           "\t"
           "\"usedStripCount\": %d,"
           "\n"
           "\t"
           "\"colorChannelOrder\": \"%s\","
           "\n"

           "\t"
           "\"opcTcpPort\": %d,"
           "\n"
           "\t"
           "\"opcUdpPort\": %d,"
           "\n"

           "\t"
           "\"enableLookupTable\": %s,"
           "\n"

           "\t"
           "\"lumCurvePower\": %.4f,"
           "\n"
           "\t"
           "\"whitePoint\": {"
           "\n"
           "\t\t"
           "\"red\": %.4f,"
           "\n"
           "\t\t"
           "\"green\": %.4f,"
           "\n"
           "\t\t"
           "\"blue\": %.4f"
           "\n"
           "\t"
           "}"
           "\n"
           "}\n",

           input_config->output_mode_name.c_str(),
           input_config->output_mapping_name.c_str(),

           input_config->leds_per_strip, input_config->used_strip_count,

           color_channel_order_to_string(input_config->color_channel_order),

           input_config->tcp_port, input_config->udp_port,

           input_config->lut_enabled ? "true" : "false",

           (double)input_config->lum_power,
           (double)input_config->white_point.red,
           (double)input_config->white_point.green,
           (double)input_config->white_point.blue);
}

class Validator {
public:
  int Validate(const server_config_t *input_config,
               std::string *diagnostic_str);

private:
  int RenderDiagnostics(std::string *diagnostic_str) {
    std::ostringstream diagnostic;
    diagnostic << "{\n";
    if (diagnostics.size() != 0) {
      diagnostic << "\t\"errors\": [";

      for (unsigned int i = 0; i < diagnostics.size(); i++) {
        if (i < diagnostics.size() - 1) {
          diagnostic << ",\n";
        } else {
          diagnostic << "\n";
        }
      }
      diagnostic << "\t],\n";
    }
    // Add closing json
    diagnostic << "\t\"valid\": " << (diagnostics.empty() ? "true" : "false")
               << "\n}";
    *diagnostic_str = diagnostic.str();
    return diagnostics.size() == 0;
  }

  void AddError(const std::string &error) { diagnostics.push_back(error); }

  void AssertEnumValid(const char *var_name, int value) {
    if (value < 0) {
      std::ostringstream os;
      os << "Invalid " << var_name;
      diagnostics.push_back(os.str());
    }
  }

  inline void AssertIntRangeInclusive(const char *var_name, int min_val,
                                      int max_val, int value) {
    if (value < min_val || value > max_val) {
      std::ostringstream os;
      os << "Given " << var_name << " (" << value << ") is outside of range "
         << min_val << "-" << max_val << " (inclusive)";
      diagnostics.push_back(os.str());
    }
  }

  inline void AssertDoubleRangeInclusive(const char *var_name, double min_val,
                                         double max_val, double value) {
    if (value < min_val || value > max_val) {
      std::ostringstream os;
      os << "Given " << var_name << " (" << value << ") is outside of range "
         << min_val << "-" << max_val << " (inclusive)";
      diagnostics.push_back(os.str());
    }
  }

  std::vector<std::string> diagnostics;
};

int validate_server_config(server_config_t *input_config,
                           std::string *diagnostic_str) {
  Validator validator;
  return validator.Validate(input_config, diagnostic_str);
}

int Validator::Validate(const server_config_t *input_config,
                        std::string *diagnostic_str) {

  { // outputMode and outputMapping
    for (int pruNum = 0; pruNum < 2; pruNum++) {
      std::string pru_file = build_pruN_program_name(
          input_config->output_mode_name.c_str(),
          input_config->output_mapping_name.c_str(), pruNum);

      if (access(pru_file.c_str(), R_OK) == -1) {
        std::ostringstream os;
        os << "Invalid mapping and/or mode name; cannot access PRU " << pruNum
           << " program '" << pru_file.c_str() << "'";
        AddError(os.str());
      }
    }
  }

  // ledsPerStrip
  AssertIntRangeInclusive("LED Count", 1, 1024, input_config->leds_per_strip);

  // usedStripCount
  AssertIntRangeInclusive("Strip/Channel Count", 1, 48,
                          input_config->used_strip_count);

  // colorChannelOrder
  AssertEnumValid("Color Channel Order", input_config->color_channel_order);

  // opcTcpPort
  AssertIntRangeInclusive("OPC TCP Port", 1, 65535, input_config->tcp_port);

  // opcUdpPort
  AssertIntRangeInclusive("OPC UDP Port", 1, 65535, input_config->udp_port);

  // e131Port
  AssertIntRangeInclusive("e131 UDP Port", 1, 65535, input_config->e131_port);

  // lumCurvePower
  AssertDoubleRangeInclusive("Luminance Curve Power", 0, 10,
                             input_config->lum_power);

  // whitePoint.red
  AssertDoubleRangeInclusive("Red White Point", 0, 1,
                             input_config->white_point.red);

  // whitePoint.green
  AssertDoubleRangeInclusive("Green White Point", 0, 1,
                             input_config->white_point.green);

  // whitePoint.blue
  AssertDoubleRangeInclusive("Blue White Point", 0, 1,
                             input_config->white_point.blue);

  return RenderDiagnostics(diagnostic_str);
}

void print_server_config(FILE *file, server_config_t *server_config) {
  char json[4096];
  server_config_to_json(json, sizeof(json), server_config);
  fputs(json, file);
}

