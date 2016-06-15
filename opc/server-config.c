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

#include "lib/cesanta/frozen.h"
#include "opc/server-error.h"
#include "opc/server-pru.h"
#include "util.h"

static const int MAX_CONFIG_FILE_LENGTH_BYTES = 1024 * 1024 * 10;

const char *demo_mode_to_string(demo_mode_t mode) {
  switch (mode) {
  case DEMO_MODE_NONE:
    return "none";
  case DEMO_MODE_FADE:
    return "fade";
  case DEMO_MODE_IDENTIFY:
    return "id";
  case DEMO_MODE_BLACK:
    return "black";
  case DEMO_MODE_POWER:
    return "power";
  default:
    return "<invalid demo_mode>";
  }
}

demo_mode_t demo_mode_from_string(const char *str) {
  if (strcasecmp(str, "none") == 0) {
    return DEMO_MODE_NONE;
  } else if (strcasecmp(str, "id") == 0) {
    return DEMO_MODE_IDENTIFY;
  } else if (strcasecmp(str, "fade") == 0) {
    return DEMO_MODE_FADE;
  } else if (strcasecmp(str, "black") == 0) {
    return DEMO_MODE_BLACK;
  } else if (strcasecmp(str, "power") == 0) {
    return DEMO_MODE_POWER;
  } else {
    return -1;
  }
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
  char *str_data = malloc(file_length + 1);
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
  char* token_value_str;
  int token_value_int;
  float token_value_float;

  if (json_size < 2) {
    // No JSON data
    return opc_server_set_error(OPC_SERVER_ERR_NO_JSON, NULL);
  }

  // Search for parameter "bar" and print it's value
  if (json_scanf(json, json_size, "outputMode:%Q", &token_value_str) > 0) {
    strlcpy(output_config->output_mode_name, token_value_str,
            sizeof(output_config->output_mode_name));
    printf("JSON outputMode %s\n", output_config->output_mode_name);
    free(token_value_str);
  }

  if (json_scanf(json, json_size, "outputMapping:%Q", &token_value_str) > 0) {
    strlcpy(output_config->output_mapping_name, token_value_str,
            sizeof(output_config->output_mapping_name));
    printf("JSON outputMapping %s\n", output_config->output_mapping_name);
    free(token_value_str);
  }

  if (json_scanf(json, json_size, "demoMode:%Q", &token_value_str) > 0) {
    output_config->demo_mode = demo_mode_from_string(token_value_str);
    printf("JSON demoMode %s\n", token_value_str);
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

  if (json_scanf(json, json_size, "enableInterpolation:%B", &token_value_int) >
      0) {
    output_config->interpolation_enabled = token_value_int;
    printf("JSON enableInterpolation %d\n", token_value_int);
  }

  if (json_scanf(json, json_size, "enableDithering:%B", &token_value_int) > 0) {
    output_config->dithering_enabled = token_value_int;
    printf("JSON enableDithering %d\n", token_value_int);
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
           "\"demoMode\": \"%s\","
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
           "\"enableInterpolation\": %s,"
           "\n"
           "\t"
           "\"enableDithering\": %s,"
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

           input_config->output_mode_name, input_config->output_mapping_name,

           demo_mode_to_string(input_config->demo_mode),

           input_config->leds_per_strip, input_config->used_strip_count,

           color_channel_order_to_string(input_config->color_channel_order),

           input_config->tcp_port, input_config->udp_port,

           input_config->interpolation_enabled ? "true" : "false",
           input_config->dithering_enabled ? "true" : "false",
           input_config->lut_enabled ? "true" : "false",

           (double)input_config->lum_power,
           (double)input_config->white_point.red,
           (double)input_config->white_point.green,
           (double)input_config->white_point.blue);
}

int validate_server_config(server_config_t *input_config,
                           char *result_json_buffer,
                           size_t result_json_buffer_size) {
  strlcpy(result_json_buffer, "{\n\t\"errors\": [", result_json_buffer_size);
  char path_temp[4096];

  int error_count = 0;

  inline void result_append(const char *format, ...) {
    snprintf(result_json_buffer + strlen(result_json_buffer),
             result_json_buffer_size - strlen(result_json_buffer) + 1, format,
             __builtin_va_arg_pack());
  }

  inline void add_error(const char *format, ...) {
    // Can't call result_append here because it breaks gcc:
    // internal compiler error: in initialize_inlined_parameters, at
    // tree-inline.c:2795
    snprintf(result_json_buffer + strlen(result_json_buffer),
             result_json_buffer_size - strlen(result_json_buffer) + 1, format,
             __builtin_va_arg_pack());
    error_count++;
  }

  inline void assert_enum_valid(const char *var_name, int value) {
    if (value < 0) {
      add_error("\n\t\t\""
                "Invalid %s"
                "\",",
                var_name);
    }
  }

  inline void assert_int_range_inclusive(const char *var_name, int min_val,
                                         int max_val, int value) {
    if (value < min_val || value > max_val) {
      add_error("\n\t\t\""
                "Given %s (%d) is outside of range %d-%d (inclusive)"
                "\",",
                var_name, value, min_val, max_val);
    }
  }

  inline void assert_double_range_inclusive(
      const char *var_name, double min_val, double max_val, double value) {
    if (value < min_val || value > max_val) {
      add_error("\n\t\t\""
                "Given %s (%f) is outside of range %f-%f (inclusive)"
                "\",",
                var_name, value, min_val, max_val);
    }
  }

  { // outputMode and outputMapping
    for (int pruNum = 0; pruNum < 2; pruNum++) {
      build_pruN_program_name(input_config->output_mode_name,
                              input_config->output_mapping_name, pruNum,
                              path_temp, sizeof(path_temp));

      if (access(path_temp, R_OK) == -1) {
        add_error("\n\t\t\""
                  "Invalid mapping and/or mode name; cannot access PRU %d "
                  "program '%s'"
                  "\",",
                  pruNum, path_temp);
      }
    }
  }

  // demoMode
  assert_enum_valid("Demo Mode", input_config->demo_mode);

  // ledsPerStrip
  assert_int_range_inclusive("LED Count", 1, 1024,
                             input_config->leds_per_strip);

  // usedStripCount
  assert_int_range_inclusive("Strip/Channel Count", 1, 48,
                             input_config->used_strip_count);

  // colorChannelOrder
  assert_enum_valid("Color Channel Order", input_config->color_channel_order);

  // opcTcpPort
  assert_int_range_inclusive("OPC TCP Port", 1, 65535, input_config->tcp_port);

  // opcUdpPort
  assert_int_range_inclusive("OPC UDP Port", 1, 65535, input_config->udp_port);

  // e131Port
  assert_int_range_inclusive("e131 UDP Port", 1, 65535,
                             input_config->e131_port);

  // lumCurvePower
  assert_double_range_inclusive("Luminance Curve Power", 0, 10,
                                input_config->lum_power);

  // whitePoint.red
  assert_double_range_inclusive("Red White Point", 0, 1,
                                input_config->white_point.red);

  // whitePoint.green
  assert_double_range_inclusive("Green White Point", 0, 1,
                                input_config->white_point.green);

  // whitePoint.blue
  assert_double_range_inclusive("Blue White Point", 0, 1,
                                input_config->white_point.blue);

  if (error_count > 0) {
    // Strip off trailing comma
    result_json_buffer[strlen(result_json_buffer) - 1] = 0;
    result_append("\n\t],\n");
  } else {
    // Reset the output to not include the error messages
    if (result_json_buffer_size > 0) {
      result_json_buffer[0] = 0;
    }
    result_append("{\n");
  }

  // Add closing json
  result_append("\t\"valid\": %s\n", error_count == 0 ? "true" : "false");
  result_append("}");

  return error_count;
}

void print_server_config(FILE* file, server_config_t* server_config) {
  char json[4096];
  server_config_to_json(json, sizeof(json), server_config);
  fputs(json, file);
}

