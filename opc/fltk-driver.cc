#include "opc/fltk-driver.h"

#include "FL/Fl_Device.H"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
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
  Fl_Offscreen buffer_;
};

FltkDriver::PixelRenderer::PixelRenderer(int W, int H)
    : Fl_Box(/* X= */ 0, /* Y= */ 0, W, H, /* L= */ nullptr), width_(W),
      height_(H) {
  box(FL_FLAT_BOX);
  color(FL_DARK2);

  buffer_ = fl_create_offscreen(width_ * EMULATED_PIXEL_SIZE,
                                height_ * EMULATED_PIXEL_SIZE);
}

void FltkDriver::PixelRenderer::SetPixelData(uint8_t *rgba_data,
                                             int num_pixels) {
  fl_begin_offscreen(buffer_);
  uint8_t* pixel = rgba_data;
  int drawn_pixels = 0;
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      uchar r = pixel[0];
      uchar g = pixel[1];
      uchar b = pixel[2];
      fl_rectf(x * EMULATED_PIXEL_SIZE, y * EMULATED_PIXEL_SIZE,
               EMULATED_PIXEL_SIZE, EMULATED_PIXEL_SIZE, r, g, b);
      pixel += 4;
      if (drawn_pixels++ > num_pixels) {
        break;
      }
    }
  }
  fl_end_offscreen();
  redraw();
}

void FltkDriver::PixelRenderer::draw() {
  //FL_Box::Draw();
  fl_copy_offscreen(0, 0, width_ * EMULATED_PIXEL_SIZE,
                    height_ * EMULATED_PIXEL_SIZE, buffer_, 0, 0);
}

FltkDriver::FltkDriver(int argc, char *argv[], int num_strips,
                       int num_pixels_per_strip)
    : num_strips_(num_strips), num_pixels_per_strip_(num_pixels_per_strip) {
  Fl_Window *window =
      new Fl_Window(num_pixels_per_strip_ * EMULATED_PIXEL_SIZE,
                    num_strips_ * EMULATED_PIXEL_SIZE);
  renderer_ = new PixelRenderer(num_pixels_per_strip, num_strips);
  window->end();
  window->show(argc, argv);

  pthread_create(&thread_handle_, NULL, &FltkDriver::ThreadFunc, this);
}

void FltkDriver::SetPixelData(uint8_t* rgba_data, int num_pixels) {
  renderer_->SetPixelData(rgba_data, num_pixels);
}

void* FltkDriver::ThreadFunc(void* this_ptr) {
  this_ptr = this_ptr;
  Fl::run();
}

