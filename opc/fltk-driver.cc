#include "opc/fltk-driver.h"

#include <iostream>
#include <math.h>
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
  PixelRenderer(int num_strips, int num_pixels_per_strip, int width,
                int height);
  void SetPixelData(buffer_pixel_t *pixels, int num_pixels);

  void draw() override;

  bool SetPixelLayoutHorizontalRectangle(int strip, int strip_offset,
                                         int num_rows, int num_columns, int x,
                                         int y, int pixel_width,
                                         int pixel_height);
  bool SetPixelLayoutVerticalRectangle(int strip, int strip_offset,
                                       int num_rows, int num_columns, int x,
                                       int y, int pixel_width,
                                       int pixel_height);
  bool SetPixelLayoutHorizontalZigzag(int strip, int strip_offset, int num_rows,
                                      int num_columns, int x, int y,
                                      int pixel_width, int pixel_height);
  bool SetPixelLayoutVerticalZigzag(int strip, int strip_offset, int num_rows,
                                    int num_columns, int x, int y,
                                    int pixel_width, int pixel_height);
  bool SetPixelLayoutLeftToRight(int strip, int strip_offset, int num_pixels,
                                 int x, int y, int pixel_width,
                                 int pixel_height);
  bool SetPixelLayoutRightToLeft(int strip, int strip_offset, int num_pixels,
                                 int x, int y, int pixel_width,
                                 int pixel_height);
  bool SetPixelLayoutTopToBottom(int strip, int strip_offset, int num_pixels,
                                 int x, int y, int pixel_width,
                                 int pixel_height);
  bool SetPixelLayoutBottomToTop(int strip, int strip_offset, int num_pixels,
                                 int x, int y, int pixel_width,
                                 int pixel_height);
  bool SetPixelLayoutLine(int strip, int strip_offset, int num_pixels, int x,
                          int y, double angle_degrees, int pixel_width,
                          int pixel_height, double pixel_step);
  bool SetPixelLayout(int strip, int strip_offset, int x, int y,
                      int pixel_width, int pixel_height);

private:
  struct PixelPosition {
    PixelPosition() {}
    PixelPosition(int x, int y, int width, int height)
        : x(x), y(y), width(width), height(height) {}
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
  };

  int num_strips_;
  int num_pixels_per_strip_;
  buffer_pixel_t *pixel_data_;
  std::vector<PixelPosition> pixel_positions_;
};

bool FltkDriver::PixelRenderer::SetPixelLayoutHorizontalRectangle(
    int strip, int strip_offset, int num_rows, int num_columns, int x, int y,
    int pixel_width, int pixel_height) {
  for (int row = 0; row < num_rows; row++) {
    if (!SetPixelLayoutLeftToRight(strip, strip_offset + row * num_columns,
                                   num_columns, x, y + row * pixel_height,
                                   pixel_width, pixel_height)) {
      std::cerr << "Error setting rectangle layout row=" << row;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutVerticalRectangle(
    int strip, int strip_offset, int num_rows, int num_columns, int x, int y,
    int pixel_width, int pixel_height) {
  for (int col = 0; col < num_columns; col++) {
    if (!SetPixelLayoutTopToBottom(strip, strip_offset + col * num_rows,
                                   num_rows, x + col * pixel_width, y,
                                   pixel_width, pixel_height)) {
      std::cerr << "Error setting rectangle layout col=" << col;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutHorizontalZigzag(
    int strip, int strip_offset, int num_rows, int num_columns, int x, int y,
    int pixel_width, int pixel_height) {
  for (int row = 0; row < num_rows; row++) {
    if (row % 2 == 0) {
      if (!SetPixelLayoutLeftToRight(strip, strip_offset + row * num_columns,
                                     num_columns, x, y + row * pixel_height,
                                     pixel_width, pixel_height)) {
        std::cerr << "Error setting zigzag layout row=" << row;
        return false;
      }
    } else {
      if (!SetPixelLayoutRightToLeft(strip, strip_offset + row * num_columns,
                                     num_columns, x, y + row * pixel_height,
                                     pixel_width, pixel_height)) {
        std::cerr << "Error setting zigzag layout row=" << row;
        return false;
      }
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutVerticalZigzag(
    int strip, int strip_offset, int num_rows, int num_columns, int x, int y,
    int pixel_width, int pixel_height) {
  for (int col = 0; col < num_columns; col++) {
    if (col % 2 == 0) {
      if (!SetPixelLayoutTopToBottom(strip, strip_offset + col * num_rows,
                                     num_rows, x + col * pixel_width, y,
                                     pixel_width, pixel_height)) {
        std::cerr << "Error setting zigzag layout col=" << col;
        return false;
      }
    } else {
      if (!SetPixelLayoutBottomToTop(strip, strip_offset + col * num_rows,
                                     num_rows, x + col * pixel_width, y,
                                     pixel_width, pixel_height)) {
        std::cerr << "Error setting zigzag layout col=" << col;
        return false;
      }
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutLine(
    int strip, int strip_offset, int num_pixels, int x, int y,
    double angle_degrees, int pixel_width, int pixel_height, double pixel_step) {
  const double angle_radians = angle_degrees * M_PI / 180;
  for (int i = 0; i < num_pixels; i++) {
    double pixel_x = x + cos(angle_radians) * pixel_step * i;
    double pixel_y = y - sin(angle_radians) * pixel_step * i;
    if (!SetPixelLayout(strip, strip_offset + i, pixel_x, pixel_y,
                        pixel_width, pixel_height)) {
      std::cerr << "Error setting left to right layout pixel=" << i;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutLeftToRight(
    int strip, int strip_offset, int num_pixels, int x, int y, int pixel_width,
    int pixel_height) {
  for (int i = 0; i < num_pixels; i++) {
    if (!SetPixelLayout(strip, strip_offset + i, x + i * pixel_width, y,
                        pixel_width, pixel_height)) {
      std::cerr << "Error setting left to right layout pixel=" << i;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutRightToLeft(
    int strip, int strip_offset, int num_pixels, int x, int y, int pixel_width,
    int pixel_height) {
  for (int i = 0; i < num_pixels; i++) {
    if (!SetPixelLayout(strip, strip_offset + num_pixels - 1 - i,
                        x + i * pixel_width, y, pixel_width, pixel_height)) {
      std::cerr << "Error setting right to left layout pixel=" << i;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutTopToBottom(
    int strip, int strip_offset, int num_pixels, int x, int y, int pixel_width,
    int pixel_height) {
  for (int i = 0; i < num_pixels; i++) {
    if (!SetPixelLayout(strip, strip_offset + i, x, y + i * pixel_height,
                        pixel_width, pixel_height)) {
      std::cerr << "Error setting top to bottom layout pixel=" << i;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayoutBottomToTop(
    int strip, int strip_offset, int num_pixels, int x, int y, int pixel_width,
    int pixel_height) {
  for (int i = 0; i < num_pixels; i++) {
    if (!SetPixelLayout(strip, strip_offset + num_pixels - 1 - i, x,
                        y + i * pixel_height, pixel_width, pixel_height)) {
      std::cerr << "Error setting bottom to top layout pixel=" << i;
      return false;
    }
  }
  return true;
}

bool FltkDriver::PixelRenderer::SetPixelLayout(int strip, int strip_offset,
                                               int x, int y, int pixel_width,
                                               int pixel_height) {
  int pixel_index = strip * num_pixels_per_strip_ + strip_offset;
  if (pixel_index < 0 || pixel_index >= num_strips_ * num_pixels_per_strip_) {
    std::cerr << "Error setting pixel layout: strip=" << strip
              << " offset=" << strip_offset << " index=" << pixel_index;
    return false;
  }
  pixel_positions_[pixel_index] =
      PixelPosition(x, y, pixel_width, pixel_height);
  return true;
}

FltkDriver::PixelRenderer::PixelRenderer(int num_strips,
                                         int num_pixels_per_strip, int width,
                                         int height)
    : Fl_Box(/* X= */ 0, /* Y= */ 0, width, height, /* L= */ nullptr),
      num_strips_(num_strips), num_pixels_per_strip_(num_pixels_per_strip),
      pixel_data_(new buffer_pixel_t[num_strips_ * num_pixels_per_strip_]),
      pixel_positions_(num_strips_ * num_pixels_per_strip_) {}

void FltkDriver::PixelRenderer::SetPixelData(buffer_pixel_t *pixels,
                                             int num_pixels) {

  memcpy(pixel_data_, pixels, num_pixels * 4);
  Fl::lock();
  redraw();
  Fl::unlock();
  Fl::awake();
}

void FltkDriver::PixelRenderer::draw() {
  for (int i = 0; i< num_strips_ * num_pixels_per_strip_; i++) {
    const PixelPosition& pos = pixel_positions_[i];
    const buffer_pixel_t& pixel = pixel_data_[i];
    fl_rectf(pos.x, pos.y, pos.width, pos.height, pixel.r,
             pixel.g, pixel.b);
  }
}

FltkDriver::FltkDriver(int argc, char *argv[], int num_strips,
                       int num_pixels_per_strip, int window_width,
                       int window_height, int default_pixel_width,
                       int default_pixel_height)
    : Driver(num_strips, num_pixels_per_strip),
      default_pixel_width_(default_pixel_width),
      default_pixel_height_(default_pixel_height) {
  Fl::lock(); // "start" the FLTK lock mechanism.
  Fl::visual(FL_RGB);
  Fl_Window *window = new Fl_Window(window_width, window_height);
  renderer_ = new PixelRenderer(num_strips, num_pixels_per_strip, window_width,
                                window_height);
  window->end();
  window->show(argc, argv);
}

bool FltkDriver::LoadLayout(const YAML::Node &layout) {
  const YAML::Node &blocks = layout["blocks"];
  if (!blocks.IsSequence()) {
    std::cerr << "Config does not have well formed blocks sequence.\n";
    return false;
  }
  for (auto it = blocks.begin(); it != blocks.end(); ++it) {
    const YAML::Node& block = *it;
    if (!LoadBlockLayout(block)) {
      std::cerr << "Error loading block " << block;
      return false;
    }
  }
  return true;
}

bool FltkDriver::LoadBlockLayout(const YAML::Node& block) {
  const std::string pattern = block["pattern"].as<std::string>();
  const int strip = block["strip"].as<int>();
  const int strip_offset = block["strip_offset"].as<int>(0);
  const int x = block["x"].as<int>();
  const int y = block["y"].as<int>();
  const int pixel_width = block["pixel_width"].as<int>();
  const int pixel_height = block["pixel_height"].as<int>();
  std::cout << "Loading layout of block " << block["name"] << " with pattern "
            << block["pattern"] << "\n";
  if (pattern == "horizontal_rectangle") {
    if (!renderer_->SetPixelLayoutHorizontalRectangle(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_rows= */ block["rows"].as<int>(),
            /* num_columns= */ block["columns"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */
            pixel_height)) {
      std::cerr << "Error setting horizontal rectangle layout\n";
      return false;
    }
  } else if (pattern == "vertical_rectangle") {
    if (!renderer_->SetPixelLayoutVerticalRectangle(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_rows= */ block["rows"].as<int>(),
            /* num_columns= */ block["columns"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting vertical rectangle layout\n";
      return false;
    }
  } else if (pattern == "horizontal_zigzag") {
    if (!renderer_->SetPixelLayoutHorizontalZigzag(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_rows= */ block["rows"].as<int>(),
            /* num_columns= */ block["columns"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting horizontal zigzag layout\n";
      return false;
    }
  } else if (pattern == "vertical_zigzag") {
    if (!renderer_->SetPixelLayoutHorizontalZigzag(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_rows= */ block["rows"].as<int>(),
            /* num_columns= */ block["columns"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting vertical zigzag layout\n";
      return false;
    }
  } else if (pattern == "left_to_right") {
    if (!renderer_->SetPixelLayoutLeftToRight(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_pixels= */ block["size"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting left to right layout\n";
      return false;
    }
  } else if (pattern == "right_to_left") {
    if (!renderer_->SetPixelLayoutRightToLeft(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_pixels= */ block["size"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting left to right layout\n";
      return false;
    }
  } else if (pattern == "top_to_bottom") {
    if (!renderer_->SetPixelLayoutTopToBottom(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_pixels= */ block["size"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting left to right layout\n";
      return false;
    }
  } else if (pattern == "bottom_to_top") {
    if (!renderer_->SetPixelLayoutBottomToTop(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_pixels= */ block["size"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height)) {
      std::cerr << "Error setting left to right layout\n";
      return false;
    }
  } else if (pattern == "line") {
    if (!renderer_->SetPixelLayoutLine(
            /* strip= */ strip,
            /* strip_offset= */ strip_offset,
            /* num_pixels= */ block["size"].as<int>(),
            /* x= */ x,
            /* y= */ y,
            /* angle_degrees= */ block["angle"].as<double>(),
            /* pixel_width= */ pixel_width,
            /* pixel_height= */ pixel_height,
            /* pixel_step= */ block["pixel_step"].as<double>())) {
      std::cerr << "Error setting left to right layout\n";
      return false;
    }
  } else {
    std::cerr << "Unknown pattern " << pattern << "\n";
    return false;
  }
  return true;
}

void FltkDriver::SetPixelData(buffer_pixel_t *pixels, int num_pixels) {
  renderer_->SetPixelData(pixels, num_pixels);
}

void FltkDriver::Run() { Fl::run(); }

