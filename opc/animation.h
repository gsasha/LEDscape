#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

#include <pthread.h>
#include <sys/time.h>

#include "opc/effect.h"
#include "opc/rate-data.h"
#include "opc/render.h"
#include "opc/server-config.h"

class StripAnimation {
public:
  void Init(int num_pixels);

  buffer_pixel_t *pixels = nullptr;
  Effect *effect = nullptr;
  bool enabled = false;
  struct timeval enable_time = {0, 0};
};

class Animation {
public:
  Animation(const server_config_t &server_config);

  void Init();
  void StartThread();
  void JoinThread();

private:
  void Thread();
  static void* ThreadFunc(void* animation_ptr);

  const server_config_t &server_config;

  RenderState render_state_;

  pthread_t thread_handle;

  StripAnimation strips_[LEDSCAPE_NUM_STRIPS];

  RateData rate_data_;
};

struct animation_state_t;
typedef struct animation_state_t animation_state_t;

animation_state_t *create_animation_state(server_config_t *server_config);

void start_animation_thread(animation_state_t* animation_state);
void join_animation_thread(animation_state_t* animation_state);

void set_animation_mode_all(animation_state_t *animation_state,
                            char *animation_mode);

void set_animation_mode_strip(animation_state_t *animation_state, int strip,
                              char *animation_mode);

void animation_disable_all(animation_state_t *animation_state, double seconds);

void animation_disable_strip(animation_state_t *animation_state, double seconds,
                             int strip);


#endif // LEDSCAPE_OPC_ANIMATION_H
