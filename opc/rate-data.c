#include "opc/rate-data.h"

void rate_data_init(struct rate_data *data, double window_size_seconds) {
  data->window_size_seconds = 0;
  gettimeofday(&data->initial_time, NULL);
  data->total_events = 0;
  data->total_rate = 0;
  gettimeofday(&data->last_window_time, NULL);
  data->last_window_events = 0;
  data->last_window_rate = 0;
}

double seconds_since(struct timeval *a, struct timeval *b) {
  double seconds_a = a->tv_sec + a->tv_usec * 1e6;
  double seconds_b = b->tv_sec + b->tv_usec * 1e6;
  return seconds_b - seconds_a;
}

int rate_data_add_event(struct rate_data *data) {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  data->total_events++;
  double seconds_since_last_window =
      seconds_since(&data->last_window_time, &current_time);
  if (seconds_since_last_window >= data->window_size_seconds) {
    double seconds_since_start =
        seconds_since(&data->initial_time, &current_time);
    data->total_rate = data->total_events / seconds_since_start;
    int last_window_intervals = data->last_window_events - 1;
    data->last_window_rate = last_window_intervals / seconds_since_last_window;

    data->last_window_time = current_time;
    data->last_window_events = 0;
    return true;
  }
  return false;
}

int rate_data_get_total_events(struct rate_data *data) {
  return data->total_events;
}

double rate_data_get_total_rate_per_sec(struct rate_data *data) {
  return data->total_rate;
}

double rate_data_get_recent_rate_per_sec(struct rate_data *data) {
  return data->last_window_rate;
}
