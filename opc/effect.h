#include "opc/render.h"

#include <vector>

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

class ColorIdentifyEffect : public Effect {
public:
  ColorIdentifyEffect(buffer_pixel_t* pixels, int num_pixels);
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

class WalkEffect : public Effect {
public:
  WalkEffect(buffer_pixel_t* pixels, int num_pixels, int offset);

  void RenderFrame() override;
private:
  const int offset_;
  int position_ = 0;
};

class ColorFadeEffect : public Effect {
public:
  ColorFadeEffect(buffer_pixel_t *pixels, int num_pixels, float offset,
                  float delta);
  void RenderFrame() override;

private:
  const float delta_;

  float H_;
  float S_ = 1.0;
  float V_ = 1.0;
};

class MatrixEffect : public Effect {
public:
  MatrixEffect(buffer_pixel_t *pixels, int num_pixels, int num_drops,
               bool forward);
  void RenderFrame() override;

private:
  void UpdateDrops();
  void RenderDrops();
  void CreateDrop(int i);

  const int num_drops_;
  const int forward_;
  const buffer_pixel_t color_ = {0, 250, 0, 0};
  std::vector<float> drops_;
  std::vector<float> speeds_;
  std::vector<int> trail_lengths_;
};

class StarsEffect : public Effect {
public:
  StarsEffect(buffer_pixel_t *pixels, int num_pixels, int num_stars);
  void RenderFrame() override;

private:
  void CreateStar(int i);

  const int num_stars_;
  const buffer_pixel_t color_ = {255, 255, 255, 0};
  std::vector<int> stars_;
  std::vector<float> luminocities_;
  std::vector<float> luminocity_limits_;
  std::vector<float> speeds_;
  std::vector<int> fading_;
};
