#ifndef LEDSCAPE_OPC_LEDSCAPE_DRIVER_H
#define LEDSCAPE_OPC_LEDSCAPE_DRIVER_H

#include "opc/driver.h"
#include "opc/server-config.h"

class LedscapeDriver : public Driver {
public:
  LedscapeDriver(const server_config_t &server_config);

  virtual void SetPixelData(uint8_t *rgba_data, int num_pixels) override;

private:
  void Init();

  const server_config_t& server_config_;
  ledscape_t *leds_ = nullptr;
};

#endif // LEDSCAPE_OPC_LEDSCAPE_DRIVER_H
