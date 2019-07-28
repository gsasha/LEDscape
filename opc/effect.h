#include "opc/render.h"

class Effect {
public:
  Effect(buffer_pixel_t *pixels, int num_pixels);
  virtual ~Effect() = default;

  virtual void RenderFrame() = 0;

protected:
  buffer_pixel_t *pixels_;
  int num_pixels_;
};

class BlackEffect : public Effect {
public:
  BlackEffect(buffer_pixel_t *pixels, int num_pixels);
  void RenderFrame() override;
};

class WhiteEffect : public Effect {
public:
  WhiteEffect(buffer_pixel_t *pixels, int num_pixels);
  void RenderFrame() override;
};

class BreatheEffect : public Effect {
public:
  BreatheEffect(buffer_pixel_t *pixels, int num_pixels);
  void RenderFrame() override;

private:
  int luminance_ = 0;
  bool up_ = true;
};
