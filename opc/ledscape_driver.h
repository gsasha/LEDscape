#ifndef LEDSCAPE_OPC_LEDSCAPE_DRIVER_H
#define LEDSCAPE_OPC_LEDSCAPE_DRIVER_H

class LedscapeDriver : public Driver {
public:
  LedscapeDriver();

  virtual void SetPixelData(uint8_t* rgba_data, int num_pixels) override;
};

#endif LEDSCAPE_OPC_LEDSCAPE_DRIVER_H
