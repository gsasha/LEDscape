#ifndef LEDSCAPE_OPC_FLTK_DRIVER_H
#define LEDSCAPE_OPC_FLTK_DRIVER_H

#include "yaml-cpp/yaml.h"
#include "opc/driver.h"

class FltkDriver : public Driver {
public:
  FltkDriver(int argc, char *argv[], int num_strips, int num_pixels_per_strip,
             int window_width, int window_height, int default_pixel_width,
             int default_pixel_height);
  bool LoadLayout(const YAML::Node& layout);
  virtual void SetPixelData(buffer_pixel_t *pixels, int num_pixels) override;

  void Run();

private:
  bool LoadBlockLayout(const YAML::Node& block);
  class PixelRenderer;

  PixelRenderer *renderer_;
  int default_pixel_width_;
  int default_pixel_height_;
};

#endif // LEDSCAPE_OPC_FLTK_DRIVER_H
