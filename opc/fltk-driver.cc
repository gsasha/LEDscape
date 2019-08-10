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

  // num_pixels is hte total number of pixels of the set.
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
    PixelPosition() : x(0), y(0) {}
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
    int pixel_pos_start, int num_pixels, int row_size, int x, int y) {
  for (int row = 0; row * row_size < num_pixels; row++) {
    SetPixelLayoutLeftToRight(pixel_pos_start + row * row_size, row_size, x,
                              y + row);
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutVerticalRectangle(
    int pixel_pos_start, int num_pixels, int col_size, int x, int y) {
  for (int col = 0; col * col_size < num_pixels; col++) {
    SetPixelLayoutTopToBottom(pixel_pos_start + col * col_size, col_size,
                              x + col, y);
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutHorizontalZigzag(
    int pixel_pos_start, int num_pixels, int row_size, int x, int y) {
  for (int row = 0; row * row_size < num_pixels; row++) {
    if (row % 2 == 0) {
      SetPixelLayoutLeftToRight(pixel_pos_start + row * row_size, row_size, x,
                                y + row);
    } else {
      SetPixelLayoutRightToLeft(pixel_pos_start + row * row_size, row_size,
                                x + row_size - 1, y + row);
    }
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutVerticalZigzag(
    int pixel_pos_start, int num_pixels, int col_size, int x, int y) {
  for (int col = 0; col * col_size < num_pixels; col++) {
    if (col % 2 == 0) {
      SetPixelLayoutTopToBottom(pixel_pos_start + col * col_size, col_size,
                                x + col, y);
    } else {
      SetPixelLayoutBottomToTop(pixel_pos_start + col * col_size, col_size,
                                x + col, y + col_size - 1);
    }
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutLeftToRight(int pixel_pos_start,
                                                          int num_pixels, int x,
                                                          int y) {
  for (int i = 0; i < num_pixels; i++) {
    pixel_positions_[pixel_pos_start + i] = {(x + i) * EMULATED_PIXEL_SIZE,
                                             y * EMULATED_PIXEL_SIZE};
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
                                                          int y) {
  for (int i = 0; i < num_pixels; i++) {
    pixel_positions_[pixel_pos_start + i] = {x * EMULATED_PIXEL_SIZE,
                                             (y + i) * EMULATED_PIXEL_SIZE};
  }
}

void FltkDriver::PixelRenderer::SetPixelLayoutBottomToTop(int pixel_pos_start,
                                                          int num_pixels, int x,
                                                          int y) {}

FltkDriver::PixelRenderer::PixelRenderer(int W, int H)
    : Fl_Box(/* X= */ 0, /* Y= */ 0, W * EMULATED_PIXEL_SIZE,
             H * EMULATED_PIXEL_SIZE, /* L= */ nullptr),
      width_(W), height_(H) {
  pixel_data_ = new buffer_pixel_t[width_ * height_];
  pixel_positions_.resize(width_ * height_);
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
  for (int i = 0; i< width_ * height_; i++) {
    const PixelPosition& pos = pixel_positions_[i];
    const buffer_pixel_t& pixel = pixel_data_[i];
    fl_rectf(pos.x, pos.y, EMULATED_PIXEL_SIZE, EMULATED_PIXEL_SIZE, pixel.r,
             pixel.g, pixel.b);
  }
  /*
  buffer_pixel_t *pixel = pixel_data_;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      fl_rectf(x * EMULATED_PIXEL_SIZE, y * EMULATED_PIXEL_SIZE,
               EMULATED_PIXEL_SIZE, EMULATED_PIXEL_SIZE, pixel->r, pixel->g,
               pixel->b);
      pixel++;
    }
  }
  */
}

FltkDriver::FltkDriver(int argc, char *argv[], int num_strips,
                       int num_pixels_per_strip)
    : Driver(num_strips, num_pixels_per_strip) {
  Fl::lock(); // "start" the FLTK lock mechanism.
  Fl::visual(FL_RGB);
  Fl_Window *window = new Fl_Window(num_pixels_per_strip * EMULATED_PIXEL_SIZE,
                                    num_strips * EMULATED_PIXEL_SIZE);
  renderer_ = new PixelRenderer(num_pixels_per_strip, num_strips);
  renderer_->SetPixelLayoutHorizontalZigzag(0, 15*15, 15, 10, 10);
  renderer_->SetPixelLayoutHorizontalZigzag(num_pixels_per_strip*40, 15*15, 15, 30, 30);
  window->end();
  window->show(argc, argv);
}

void FltkDriver::SetPixelData(buffer_pixel_t *pixels, int num_pixels) {
  renderer_->SetPixelData(pixels, num_pixels);
}

void FltkDriver::Run() { Fl::run(); }

