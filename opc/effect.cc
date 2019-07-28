#include "opc/effect.h"

#include <cstdio>
#include <cstring>

Effect::Effect(buffer_pixel_t *pixels, int num_pixels)
    : pixels_(pixels), num_pixels_(num_pixels) {}

BlackEffect::BlackEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void BlackEffect::RenderFrame() {
  memset(pixels_, 0, num_pixels_ * 4);
}

WhiteEffect::WhiteEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void WhiteEffect::RenderFrame() {
  memset(pixels_, 0xff, num_pixels_ * 4);
}

BreatheEffect::BreatheEffect(buffer_pixel_t *pixels, int num_pixels)
    : Effect(pixels, num_pixels) {}

void BreatheEffect::RenderFrame() {
  memset(pixels_, luminance_, num_pixels_ * sizeof(buffer_pixel_t));
  if (up_) {
    luminance_+= 1;
    if (luminance_ == 250) {
      up_ = false;
    }
  } else {
    luminance_-=1;
    if (luminance_ == 0) {
      up_ = true;
    }
  }
  //fprintf(stderr, "Render breathe %d\n", luminance_+100);
}
