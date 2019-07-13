#include "opc/server-error.h"

#include <stdarg.h>
#include <string.h>

__thread opc_error_code_t g_error_code = OPC_SERVER_ERR_NONE;
__thread char g_error_info_str[4096] = {0};

const char *opc_server_strerr(opc_error_code_t error_code) {
  switch (error_code) {
  case OPC_SERVER_ERR_NONE:
    return "No error";
  case OPC_SERVER_ERR_NO_JSON:
    return "No JSON document given";
  case OPC_SERVER_ERR_INVALID_JSON:
    return "Invalid JSON document given";
  default:
    return "Unkown Error";
  }
}

int opc_server_set_error_av(opc_error_code_t error_code, const char *extra_info,
                            va_list ap) {
  g_error_code = error_code;

  char extra_info_out[2048];
  va_list ap_copy;
  va_copy(ap_copy, ap);
  int len =
      vsnprintf(extra_info_out, sizeof(extra_info_out), extra_info, ap_copy);
  va_end(ap_copy);
  if (len < 0) {
    strcpy(extra_info_out, "failed formatting");
  }

  snprintf(g_error_info_str, sizeof(g_error_info_str), "%s: %s",
           opc_server_strerr(error_code), extra_info_out);

  return -1;
}

int opc_server_set_error(opc_error_code_t error_code, const char *extra_info,
                         ...) {
  va_list ap;
  va_start(ap, extra_info);
  int ret = opc_server_set_error(error_code, extra_info, ap);
  va_end(ap);
  return ret;
}

