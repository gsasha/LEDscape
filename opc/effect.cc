#include "opc/effect.h"

#include <cmath>
#include <cstdio>
#include <cstring>

Effect::Effect(buffer_pixel_t *pixels, int num_pixels)
    : pixels_(pixels), num_pixels_(num_pixels) {}

BlackEffect::BlackEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void BlackEffect::RenderFrame() { memset(pixels_, 0, num_pixels_ * 4); }

WhiteEffect::WhiteEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void WhiteEffect::RenderFrame() { memset(pixels_, 0xff, num_pixels_ * 4); }

ColorIdentifyEffect::ColorIdentifyEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void ColorIdentifyEffect::RenderFrame() {
  memset(pixels_, 0, num_pixels_ * 4);
  // x red, 1 black, x green, 1 white, x blue, 1 black, x w.
  int pos = 0;
  int x = 5;
  for (int i = 0; i < x; i++, pos++) {
    pixels_[pos].r = 0xff;
  }
  pixels_[pos].r = 0x0;
  pixels_[pos].g = 0x0;
  pixels_[pos].b = 0x0;
  pixels_[pos].w = 0x0;
  pos++;

  for (int i = 0; i < x; i++, pos++) {
    pixels_[pos].g = 0xff;
  }
  pixels_[pos].r = 0x0;
  pixels_[pos].g = 0x0;
  pixels_[pos].b = 0x0;
  pixels_[pos].w = 0x0;
  pos++;

  for (int i = 0; i < x; i++, pos++) {
    pixels_[pos].b = 0xff;
  }
  pixels_[pos].r = 0x0;
  pixels_[pos].g = 0x0;
  pixels_[pos].b = 0x0;
  pixels_[pos].w = 0x0;
  pos++;

  for (int i = 0; i < x; i++, pos++) {
    pixels_[pos].w = 0xff;
  }
  pixels_[pos].r = 0xff;
  pixels_[pos].g = 0xff;
  pixels_[pos].b = 0xff;
  pixels_[pos].w = 0xff;
  pos++;
}

BreatheEffect::BreatheEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void BreatheEffect::RenderFrame() {
  memset(pixels_, luminance_, num_pixels_ * sizeof(buffer_pixel_t));
  if (up_) {
    luminance_ += 1;
    if (luminance_ == 250) {
      up_ = false;
    }
  } else {
    luminance_ -= 1;
    if (luminance_ == 0) {
      up_ = true;
    }
  }
}

WalkEffect::WalkEffect(buffer_pixel_t *pixels, int num_pixels, int offset)
    : Effect(pixels, num_pixels), offset_(offset), position_(offset) {}

void WalkEffect::RenderFrame() {
  memset(pixels_, 0, num_pixels_ * sizeof(buffer_pixel_t));
  *(uint32_t *)&pixels_[position_] = 0xffffffff;
  position_ += 77 + offset_;
  position_ %= num_pixels_;
}

void HSV_to_RGB(float h, float s, float v, float *r, float *g, float *b) {
  float c = v * s;
  float x = c * (1.0 - fabs(fmod(h / 60.0, 2) - 1.0));
  float m = v - c;
  if (h >= 0.0 && h < 60.0) {
    *r = c + m;
    *g = x + m;
    *b = m;
  } else if (h >= 60.0 && h < 120.0) {
    *r = x + m;
    *g = c + m;
    *b = m;
  } else if (h >= 120.0 && h < 180.0) {
    *r = m;
    *g = c + m;
    *b = x + m;
  } else if (h >= 180.0 && h < 240.0) {
    *r = m;
    *g = x + m;
    *b = c + m;
  } else if (h >= 240.0 && h < 300.0) {
    *r = x + m;
    *g = m;
    *b = c + m;
  } else if (h >= 300.0 && h < 360.0) {
    *r = c + m;
    *g = m;
    *b = x + m;
  } else {
    *r = m;
    *g = m;
    *b = m;
  }
}

ColorFadeEffect::ColorFadeEffect(buffer_pixel_t *pixels, int num_pixels,
                                 float offset, float delta)
    : Effect(pixels, num_pixels), delta_(delta), H_(offset) {}

void ColorFadeEffect::RenderFrame() {
  float r, g, b;
  HSV_to_RGB(H_, S_, V_, &r, &g, &b);
  H_ += delta_;
  if (H_ >= 360) {
    H_ = 0;
  }

  buffer_pixel_t color;
  color.r = r * 250;
  color.g = g * 250;
  color.b = b * 250;
  uint32_t color_int = *(uint32_t *)&color;
  for (int i = 0; i < num_pixels_; i++) {
    *(uint32_t *)&pixels_[i] = color_int;
  }
}

MatrixEffect::MatrixEffect(buffer_pixel_t *pixels, int num_pixels,
                           int num_drops, bool forward)
    : Effect(pixels, num_pixels), num_drops_(num_drops), forward_(forward),
      drops_(num_drops_, 0.0), speeds_(num_drops_, 0.0),
      trail_lengths_(num_drops_, 0.0) {
  for (int i = 0; i < num_drops_; i++) {
    CreateDrop(i);
  }
}

void MatrixEffect::RenderFrame() {
  UpdateDrops();
  RenderDrops();
}

void MatrixEffect::CreateDrop(int i) {
  if (forward_) {
    drops_[i] = (rand() % num_pixels_) / 4.0;
  } else {
    drops_[i] = num_pixels_ - (rand() % num_pixels_) / 4.0;
  }
  speeds_[i] = exp(-(rand() % 100 / 50.0));
  trail_lengths_[i] = (rand() % num_pixels_) / 10. + 1;
}

void MatrixEffect::UpdateDrops() {
  if (forward_) {
    for (int i = 0; i < num_drops_; ++i) {
      drops_[i] += speeds_[i];
      if (drops_[i] - trail_lengths_[i] > num_pixels_) {
        CreateDrop(i);
      }
    }
  } else {
    for (int i = 0; i < num_drops_; ++i) {
      drops_[i] -= speeds_[i];
      if (drops_[i] + trail_lengths_[i] < 0) {
        CreateDrop(i);
      }
    }
  }
}

void MatrixEffect::RenderDrops() {
  memset(pixels_, 0, num_pixels_ * sizeof(buffer_pixel_t));
  if (forward_) {
    for (int i = 0; i < num_drops_; i++) {
      buffer_pixel_t color = color_;
      buffer_pixel_t color_diff = color_;
      color_diff.r /= trail_lengths_[i];
      color_diff.g /= trail_lengths_[i];
      color_diff.b /= trail_lengths_[i];
      int stop = drops_[i] - trail_lengths_[i];
      for (int p = drops_[i]; p > stop; p--) {
        color.r -= color_diff.r;
        color.g -= color_diff.g;
        color.b -= color_diff.b;
        if (p >= 0 && p < num_pixels_) {
          pixels_[p] = color;
        }
      }
    }
  } else {
    for (int i = 0; i < num_drops_; i++) {
      buffer_pixel_t color = color_;
      buffer_pixel_t color_diff = color_;
      color_diff.r /= trail_lengths_[i];
      color_diff.g /= trail_lengths_[i];
      color_diff.b /= trail_lengths_[i];
      int stop = drops_[i] + trail_lengths_[i];
      for (int p = drops_[i]; p < stop; p++) {
        color.r -= color_diff.r;
        color.g -= color_diff.g;
        color.b -= color_diff.b;
        if (p >= 0 && p < num_pixels_) {
          pixels_[p] = color;
        }
      }
    }
  }
}

StarsEffect::StarsEffect(buffer_pixel_t *pixels, int num_pixels, int num_stars)
    : Effect(pixels, num_pixels), num_stars_(num_stars), stars_(num_stars_),
      luminocities_(num_stars_), luminocity_limits_(num_stars),
      speeds_(num_stars_), fading_(num_stars_) {
  for (int i = 0; i < num_stars_; i++) {
    CreateStar(i);
  }
}

void StarsEffect::CreateStar(int i) {
  stars_[i] = rand() % num_pixels_;
  speeds_[i] = exp(3.0 - (rand() % 100 / 10.0));
  luminocity_limits_[i] = speeds_[i] / exp(3.0) * (rand() % 50 + 200);
  luminocities_[i] = rand() % ((int)(luminocity_limits_[i] / 4 + 1));
  fading_[i] = false;
}

void StarsEffect::RenderFrame() {
  for (int i = 0; i < num_stars_; i++) {
    if (fading_[i]) {
      luminocities_[i] -= speeds_[i];
      if (luminocities_[i] <= 0) {
        CreateStar(i);
      }
    } else {
      luminocities_[i] += speeds_[i];
      if (luminocities_[i] >= luminocity_limits_[i]) {
        fading_[i] = true;
      }
    }
  }

  memset(pixels_, 0, num_pixels_ * sizeof(buffer_pixel_t));
  for (int i = 0; i < num_stars_; i++) {
    buffer_pixel_t color;
    color.r = color.g = color.b = luminocities_[i];
    pixels_[stars_[i]] = color;
  }
}
