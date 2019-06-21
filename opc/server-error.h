#ifndef LEDSCAPE_OPC_SERVER_ERROR_H
#define LEDSCAPE_OPC_SERVER_ERROR_H

#include <stdio.h>

// Error Handling

typedef enum {
  OPC_SERVER_ERR_NONE,
  OPC_SERVER_ERR_NO_JSON,
  OPC_SERVER_ERR_INVALID_JSON,
  OPC_SERVER_ERR_FILE_READ_FAILED,
  OPC_SERVER_ERR_FILE_WRITE_FAILED,
  OPC_SERVER_ERR_FILE_TOO_LARGE,
  OPC_SERVER_ERR_SEEK_FAILED
} opc_error_code_t;

extern __thread opc_error_code_t g_error_code;
extern __thread char g_error_info_str[4096];

const char *opc_server_strerr(opc_error_code_t error_code);

int opc_server_set_error(opc_error_code_t error_code,
                         const char* extra_info, ...);

#endif // LEDSCAPE_OPC_SERVER_ERROR_H
