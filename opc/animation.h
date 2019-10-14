#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

#include <pthread.h>
#include <sys/time.h>

#include <memory>
#include <vector>

#include "opc/driver.h"
#include "opc/effect.h"
#include "opc/rate-data.h"
#include "opc/render.h"
#include "yaml-cpp/yaml.h"

class StripAnimation {
public:
  void Init(int num_pixels);

  buffer_pixel_t *pixels = nullptr;
  std::unique_ptr<Effect> effect;
  bool enabled = false;
  struct timeval enable_time = {0, 0};
};

class Animation {
public:
  Animation(Driver* driver);

  void Init();
  bool SetScenario(const YAML::Node& scenario);
  void StartThread();
  void JoinThread();

private:
  void Thread();
  static void* ThreadFunc(void* animation_ptr);

  void SetDefaultScenario();
  void SetEffect(int strip, const YAML::Node& effect);
  void SetEffectLocked(int strip, const YAML::Node& effect);

  Driver* driver_;
  RenderState render_state_;

  pthread_t thread_handle;

  pthread_mutex_t animation_mutex_;
  std::vector<StripAnimation> strips_;

  RateData rate_data_;
};

#endif // LEDSCAPE_OPC_ANIMATION_H
