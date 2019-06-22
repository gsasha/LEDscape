#ifndef LEDSCAPE_OPC_RATE_DATA_H
#define LEDSCAPE_OPC_RATE_DATA_H

#include <sys/time.h>
#include <time.h>

struct rate_data {
  double window_size_seconds;
  struct timeval initial_time;
  int total_events;
  double total_rate;
  struct timeval last_window_time;
  int last_window_events;
  double last_window_rate;
};

void rate_data_init(struct rate_data*, double window_size_seconds);
// returns true if a new value of window event is available.
int rate_data_add_event(struct rate_data*);
int rate_data_get_total_events(struct rate_data*);
double rate_data_get_total_rate_per_sec(struct rate_data*);
double rate_data_get_recent_rate_per_sec(struct rate_data*);

#endif // LEDSCAPE_OPC_RATE_DATA_H
