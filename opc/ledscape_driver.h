#ifndef LEDSCAPE_OPC_LEDSCAPE_DRIVER_H
#define LEDSCAPE_OPC_LEDSCAPE_DRIVER_H

#include "opc/driver.h"
#include "opc/server-config.h"

class LedscapeDriver : public Driver {
public:
  LedscapeDriver(const server_config_t &server_config);

  virtual void SetPixelData(buffer_pixel_t *pixels, int num_pixels) override;

private:
  void Init();
  void BuildLookupTables();

  const server_config_t& server_config_;
  ledscape_t *leds_ = nullptr;

  uint8_t lut_lookup_red_[257];
  uint8_t lut_lookup_green_[257];
  uint8_t lut_lookup_blue_[257];
  bool lut_enabled_ = false;
};

#endif // LEDSCAPE_OPC_LEDSCAPE_DRIVER_H
