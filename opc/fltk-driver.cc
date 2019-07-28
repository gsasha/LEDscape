#include "opc/fltk-driver.h"

#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

// Size of a single emulated pixel as shown on the screen.
constexpr int EMULATED_PIXEL_SIZE = 10;

class FltkDriver::PixelRenderer : public Fl_Box {
public:
  PixelRenderer(int W, int H);
  void SetPixelData(uint8_t *rgba_data, int num_pixels);

  void draw() override;

private:
  int width_;
  int height_;
  uint8_t* pixel_data_;
};

FltkDriver::PixelRenderer::PixelRenderer(int W, int H)
    : Fl_Box(/* X= */ 0, /* Y= */ 0, W, H, /* L= */ nullptr), width_(W),
      height_(H) {
  box(FL_FLAT_BOX);
  color(FL_DARK2);
  pixel_data_ = new uint8_t[width_ * height_ * 4];
}

void FltkDriver::PixelRenderer::SetPixelData(uint8_t *rgba_data,
                                             int num_pixels) {

  memcpy(pixel_data_, rgba_data, num_pixels * 4);
  Fl::lock();
  parent()->redraw();
  Fl::flush();
  Fl::unlock();
  Fl::awake();
}

void FltkDriver::PixelRenderer::draw() {
  uint8_t* pixel = pixel_data_;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      uchar r = pixel[0];
      uchar g = pixel[1];
      uchar b = pixel[2];
      fl_rectf(x * EMULATED_PIXEL_SIZE, y * EMULATED_PIXEL_SIZE,
               EMULATED_PIXEL_SIZE, EMULATED_PIXEL_SIZE, r, g, b);
      pixel += 4;
    }
  }
}

FltkDriver::FltkDriver(int argc, char *argv[], int num_strips,
                       int num_pixels_per_strip)
    : Driver(num_strips, num_pixels_per_strip) {
  Fl::visual(FL_RGB);
  Fl_Window *window =
      new Fl_Window(num_pixels_per_strip * EMULATED_PIXEL_SIZE,
                    num_strips * EMULATED_PIXEL_SIZE);
  renderer_ = new PixelRenderer(num_pixels_per_strip, num_strips);
  window->end();
  window->show(argc, argv);
}

void FltkDriver::SetPixelData(uint8_t *rgba_data, int num_pixels) {
  renderer_->SetPixelData(rgba_data, num_pixels);
}

void FltkDriver::Run() {
  Fl::run();
}

