#ifndef LEDSCAPE_OPC_SERVER_ERROR_H
#define LEDSCAPE_OPC_SERVER_ERROR_H

#include <string>

// Error Handling

enum opc_error_code_t {
  OPC_SERVER_ERR_NONE,
  OPC_SERVER_ERR_NO_JSON,
  OPC_SERVER_ERR_INVALID_JSON,
  OPC_SERVER_ERR_FILE_READ_FAILED,
  OPC_SERVER_ERR_FILE_WRITE_FAILED,
  OPC_SERVER_ERR_FILE_TOO_LARGE,
  OPC_SERVER_ERR_SEEK_FAILED
};

int opc_server_set_error(opc_error_code_t error_code,
                         const char* extra_info, ...);

std::string opc_server_get_error();

#endif // LEDSCAPE_OPC_SERVER_ERROR_H
