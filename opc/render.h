#ifndef LEDSCAPE_OPC_RENDER_H
#define LEDSCAPE_OPC_RENDER_H

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include "ledscape/ledscape.h"
#include "opc/rate-data.h"
#include "opc/server-config.h"

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t unused;
} __attribute__((__packed__)) buffer_pixel_t;

class RenderState {
public:
  RenderState(const server_config_t &server_config);

  void StartThread();
  void JoinThread();

  void SetStripData(int strip, buffer_pixel_t *strip_data,
                    int strip_num_pixels);

private:
  void BuildLookupTables(const server_config_t &server_config);
  void InitLedscape(const server_config_t &server_config);
  static void *ThreadFunc(void *render_state);
  void Thread();
  void RenderBackingData();

  int used_strip_count;
  int leds_per_strip;
  int num_leds;
  color_channel_order_t color_channel_order;

  pthread_t thread_handle;
  pthread_mutex_t frame_data_mutex;
  buffer_pixel_t *frame_data;
  buffer_pixel_t *backing_data;

  ledscape_t *leds;

  uint8_t lut_lookup_red[257];
  uint8_t lut_lookup_green[257];
  uint8_t lut_lookup_blue[257];
  bool lut_enabled;

  RateData rate_data;
};

#endif // LEDSCAPE_OPC_RENDER_H
