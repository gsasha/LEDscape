#include "opc/fltk-driver.h"

#include <iostream>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

// Size of a single emulated pixel as shown on the screen.
constexpr int EMULATED_PIXEL_SIZE = 16;

class FltkDriver::PixelRenderer : public Fl_Box {
public:
  PixelRenderer(int W, int H);
  void SetPixelData(buffer_pixel_t *pixels, int num_pixels);

  void draw() override;

  void SetPixelLayoutHorizontalRectangle(int pixel_pos_start, int num_pixels,
                                         int row_size, int x, int y);
  void SetPixelLayoutVerticalRectangle(int pixel_pos_start, int num_pixels,
                                       int column_size, int x, int y);
  void SetPixelLayoutHorizontalZigzag(int pixel_pos_start, int num_pixels,
                                      int row_size, int x, int y);
  void SetPixelLayoutVerticalZigzag(int pixel_pos_start, int num_pixels,
                                    int column_size, int x, int y);
  void SetPixelLayoutLeftToRight(int pixel_pos_start, int num_pixels, int x,
                                 int y);
  void SetPixelLayoutRightToLeft(int pixel_pos_start, int num_pixels, int x,
                                 int y);
  void SetPixelLayoutTopToBottom(int pixel_pos_start, int num_pixels, int x,
                                 int y);
  void SetPixelLayoutBottomToTop(int pixel_pos_start, int num_pixels, int x,
                                 int y);

private:
  struct PixelPosition {
    PixelPosition(int x, int y) : x(x), y(y) {}
    int x = 0;
    int y = 0;
  };

  int width_;
  int height_;
  buffer_pixel_t *pixel_data_;
  std::vector<PixelPosition> pixel_positions_;
};

void FltkDriver::PixelRenderer::SetPixelLayoutHorizontalRectangle(
    int pixel_pos_start, int num_pixels, int row_size, int x, int y) {}

void FltkDriver::PixelRenderer::SetPixelLayoutVerticalRectangle(
    int pixel_pos_start, int num_pixels, int column_size, int x, int y) {}

void FltkDriver::PixelRenderer::SetPixelLayoutHorizontalZigzag(
    int pixel_pos_start, int num_pixels, int row_size, int x, int y) {}

void FltkDriver::PixelRenderer::SetPixelLayoutVerticalZigzag(
    int pixel_pos_start, int num_pixels, int column_size, int x, int y) {}

void FltkDriver::PixelRenderer::SetPixelLayoutLeftToRight(int pixel_pos_start,
                                                          int num_pixels, int x,
                                                          int y) {
  for (int i = 0; i < num_pixels; i++) {
    pixel_positions_[pixel_pos_start + i] =
        PixelPosition((x + i) * EMULATED_PIXEL_SIZE, y * EMULATED_PIXEL_SIZE);
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutRightToLeft(int pixel_pos_start,
                                                          int num_pixels, int x,
                                                          int y) {
  for (int i = 0; i < num_pixels; i++) {
    pixel_positions_[pixel_pos_start + i] = {(x - i) * EMULATED_PIXEL_SIZE,
                                             y * EMULATED_PIXEL_SIZE};
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutTopToBottom(int pixel_pos_start,
                                                          int num_pixels, int x,
                                                          int y) {}

void FltkDriver::PixelRenderer::SetPixelLayoutBottomToTop(int pixel_pos_start,
                                                          int num_pixels, int x,
                                                          int y) {}

FltkDriver::PixelRenderer::PixelRenderer(int W, int H)
    : Fl_Box(/* X= */ 0, /* Y= */ 0, W * EMULATED_PIXEL_SIZE,
             H * EMULATED_PIXEL_SIZE, /* L= */ nullptr),
      width_(W), height_(H) {
  pixel_data_ = new buffer_pixel_t[width_ * height_];
}

void FltkDriver::PixelRenderer::SetPixelData(buffer_pixel_t *pixels,
                                             int num_pixels) {

  memcpy(pixel_data_, pixels, num_pixels * 4);
  Fl::lock();
  redraw();
  Fl::unlock();
  Fl::awake();
}

void FltkDriver::PixelRenderer::draw() {
  buffer_pixel_t *pixel = pixel_data_;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      fl_rectf(x * EMULATED_PIXEL_SIZE, y * EMULATED_PIXEL_SIZE,
               EMULATED_PIXEL_SIZE, EMULATED_PIXEL_SIZE, pixel->r, pixel->g,
               pixel->b);
      pixel++;
    }
  }
}

FltkDriver::FltkDriver(int argc, char *argv[], int num_strips,
                       int num_pixels_per_strip)
    : Driver(num_strips, num_pixels_per_strip) {
  Fl::lock(); // "start" the FLTK lock mechanism.
  Fl::visual(FL_RGB);
  Fl_Window *window = new Fl_Window(num_pixels_per_strip * EMULATED_PIXEL_SIZE,
                                    num_strips * EMULATED_PIXEL_SIZE);
  renderer_ = new PixelRenderer(num_pixels_per_strip, num_strips);
  window->end();
  window->show(argc, argv);
}

void FltkDriver::SetPixelData(buffer_pixel_t *pixels, int num_pixels) {
  renderer_->SetPixelData(pixels, num_pixels);
}

void FltkDriver::Run() { Fl::run(); }

