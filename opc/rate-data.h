#ifndef LEDSCAPE_OPC_RATE_DATA_H
#define LEDSCAPE_OPC_RATE_DATA_H

#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

struct rate_scheduler_t {
  struct timeval frame_tv;
  struct timeval step_frame_tv;
};

void init_rate_scheduler(struct rate_scheduler_t *rate_scheduler,
                         int times_per_second);
void rate_scheduler_wait_frame(struct rate_scheduler_t *rate_scheduler);

struct rate_data_t {
  double window_size_seconds;
  struct timeval initial_time;
  int total_events;
  double total_rate;
  struct timeval last_window_time;
  int last_window_events;
  double last_window_rate;
};

void init_rate_data(struct rate_data_t*, double window_size_seconds);
// returns true if a new value of window event is available.
bool rate_data_add_event(struct rate_data_t*);
int rate_data_get_total_events(struct rate_data_t*);
double rate_data_get_total_rate_per_sec(struct rate_data_t*);
double rate_data_get_recent_rate_per_sec(struct rate_data_t*);

#endif // LEDSCAPE_OPC_RATE_DATA_H
