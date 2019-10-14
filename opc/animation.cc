#include "opc/animation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// use this hacky homegrown implementation until the compiler supports
// std::make_unique out of the box.
namespace hack {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}

void StripAnimation::Init(int num_pixels) {
  pixels = (buffer_pixel_t *)malloc(num_pixels * sizeof(buffer_pixel_t));
  memset((void *)pixels, 0, num_pixels * sizeof(buffer_pixel_t));
  gettimeofday(&enable_time, NULL);
}

Animation::Animation(Driver *driver)
    : driver_(driver), render_state_(driver), rate_data_(10) {
  Init();
}

void Animation::Init() {
  strips_.resize(driver_->num_strips());
  pthread_mutex_init(&animation_mutex_, NULL);

  for (int i = 0; i < driver_->num_strips(); i++) {
    strips_[i].Init(driver_->num_pixels_per_strip());
  }
  SetDefaultScenario();
}

bool Animation::SetScenario(const YAML::Node &scenario) {
  std::map<int, const YAML::Node*> effects;
  const YAML::Node &default_effect = scenario["default_effect"];
  const YAML::Node &effects_node = scenario["effects"];
  if (effects_node.IsSequence()) {
    for (const YAML::Node &effect_node : effects_node) {
      const YAML::Node &strips = effect_node["strips"];
      for (const YAML::Node &strip : strips) {
        effects[strip.as<int>()] = &effect_node;
      }
    }
  }

  pthread_mutex_lock(&animation_mutex_);
  for (int strip = 0; strip < driver_->num_strips(); strip++) {
    const YAML::Node &effect =
        effects.find(strip) == effects.end() ? default_effect : *effects[strip];
    if (!effect.IsDefined()) {
      continue;
    } else {
      SetEffectLocked(strip, effect);
    }
  }
  pthread_mutex_unlock(&animation_mutex_);
  return true;
}

void Animation::SetEffect(int strip, const YAML::Node& effect) {
  pthread_mutex_lock(&animation_mutex_);
  SetEffectLocked(strip, effect);
  pthread_mutex_unlock(&animation_mutex_);
}

void Animation::SetEffectLocked(int strip, const YAML::Node& effect) {
  const std::string mode = effect["mode"].as<std::string>("black");
  auto* pixels = strips_[strip].pixels;
  const auto num_pixels = driver_->num_pixels_per_strip();
  
  if (mode == "black") {
    strips_[strip].effect = hack::make_unique<BlackEffect>(pixels, num_pixels);
  } else if (mode == "white") {
    strips_[strip].effect = hack::make_unique<WhiteEffect>(pixels, num_pixels);
  } else if (mode == "stars") {
    int num_stars = effect["amount"].as<int>(0) +
                    rand() % effect["random_amount"].as<int>(1);
    strips_[strip].effect =
        hack::make_unique<StarsEffect>(pixels, num_pixels, num_stars);
  } else if (mode == "matrix") {
    int num_drops = effect["num_drops"].as<int>(5);
    bool forward = effect["direction"].as<std::string>("forward") == "forward";
    strips_[strip].effect =
        hack::make_unique<MatrixEffect>(pixels, num_pixels, num_drops, forward);
  } else if (mode == "breathe") {
    strips_[strip].effect = hack::make_unique<BreatheEffect>(pixels, num_pixels);
  } else if (mode == "color_diagnostic") {
    strips_[strip].effect =
        hack::make_unique<ColorDiagnosticEffect>(pixels, num_pixels);
  } else {
    strips_[strip].effect =
        hack::make_unique<ColorDiagnosticEffect>(pixels, num_pixels);
  }
}

void Animation::SetDefaultScenario() {
  pthread_mutex_lock(&animation_mutex_);

  for (int i = 0; i < driver_->num_strips(); i++) {
    auto *pixels = strips_[i].pixels;
    const auto num_pixels = driver_->num_pixels_per_strip();

    if (i < 4) {
      strips_[i].effect =
          hack::make_unique<ColorDiagnosticEffect>(pixels, num_pixels);
    } else if (i < 24) {
      strips_[i].effect =
          hack::make_unique<StarsEffect>(pixels, num_pixels, i * 5 + 15);
    } else if (i < 25) {
      strips_[i].effect = hack::make_unique<BreatheEffect>(pixels, num_pixels);
    } else if (i < 30) {
      strips_[i].effect =
          hack::make_unique<WalkEffect>(pixels, num_pixels, rand() % 100);
    } else if (i < 40) {
      int offset = rand() % 4 + i;
      double step = (rand() % 1000) / 100000.0;
      strips_[i].effect =
          hack::make_unique<ColorFadeEffect>(pixels, num_pixels, offset, step);
    } else {
      strips_[i].effect =
          hack::make_unique<MatrixEffect>(pixels, num_pixels, 5, true);
    }
  }
  pthread_mutex_unlock(&animation_mutex_);
}

void Animation::StartThread() {
  pthread_create(&thread_handle, NULL, &Animation::ThreadFunc, this);
  render_state_.StartThread();
}

void Animation::JoinThread() { pthread_join(thread_handle, NULL); }

void Animation::Thread() {
  RateScheduler rate_scheduler(60);

  for (;;) {
    rate_scheduler.WaitFrame();

    struct timeval now;
    gettimeofday(&now, NULL);

    pthread_mutex_lock(&animation_mutex_);

    // TODO(gsasha): add thread cancellation?
    for (int strip_index = 0; strip_index < driver_->num_strips();
         strip_index++) {
      auto *strip = &strips_[strip_index];
      if (!strip->enabled) {
        if (timercmp(&now, &strip->enable_time, >)) {
          // Strip will start executing on next round.
          strip->enabled = true;
        }
        continue;
      }
      // Render this strip.
      if (strip->effect != nullptr) {
        strip->effect->RenderFrame();
      } else {
        if (strip_index % 2 == 1) {
          memset(strip->pixels, 0xaa,
                 driver_->num_pixels_per_strip() * sizeof(buffer_pixel_t));
        }
      }
      render_state_.SetStripData(strip_index, strip->pixels,
                                 driver_->num_pixels_per_strip());
    }
    pthread_mutex_unlock(&animation_mutex_);

    if (rate_data_.AddEvent()) {
      printf("[animation] frames %d, rate %lf fps, recent rate %lf fps\n",
             rate_data_.GetTotalEvents(),
             rate_data_.GetTotalRatePerSec(),
             rate_data_.GetRecentRatePerSec());
    }
  }
}

void *Animation::ThreadFunc(void *animation_ptr) {
  auto *animation_state = static_cast<Animation *>(animation_ptr);
  animation_state->Thread();
  return nullptr;
}


