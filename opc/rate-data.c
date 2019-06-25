#include "opc/rate-data.h"

#include <stdio.h>
#include <unistd.h>

void timeval_add(struct timeval *dst, struct timeval *added) {
  dst->tv_usec += added->tv_usec;
  if (dst->tv_usec > 1000000) {
    dst->tv_usec -= 1000000;
    dst->tv_sec++;
  }
  dst->tv_sec += added->tv_sec;
}

// return true if a<b.
int timeval_lt(struct timeval *a, struct timeval *b) {
  if (a->tv_sec < b->tv_sec) {
    return true;
  }
  if (a->tv_sec > b->tv_sec) {
    return false;
  }
  return a->tv_usec < b->tv_usec;
}

// return microseconds of b-a;
int timeval_microseconds_until(struct timeval *a, struct timeval *b) {
  return (b->tv_sec - a->tv_sec) * 1000000 + b->tv_usec - a->tv_usec;
}

void init_rate_scheduler(struct rate_scheduler_t *rate_scheduler,
                         int times_per_second) {
  gettimeofday(&rate_scheduler->frame_tv, NULL);
  rate_scheduler->step_frame_tv.tv_sec = 0;
  rate_scheduler->step_frame_tv.tv_usec = 1000000 / times_per_second;
}

void rate_scheduler_wait_frame(struct rate_scheduler_t *rate_scheduler) {
  timeval_add(&rate_scheduler->frame_tv, &rate_scheduler->step_frame_tv);

  struct timeval current_time_tv;
  gettimeofday(&current_time_tv, NULL);

  if (timeval_lt(&current_time_tv, &rate_scheduler->frame_tv)) {
    usleep(timeval_microseconds_until(&current_time_tv,
                                      &rate_scheduler->frame_tv));
  }
}

void init_rate_data(struct rate_data_t *data, double window_size_seconds) {
  data->window_size_seconds = window_size_seconds;
  gettimeofday(&data->initial_time, NULL);
  data->total_events = 0;
  data->total_rate = 0;
  gettimeofday(&data->last_window_time, NULL);
  data->last_window_events = 0;
  data->last_window_rate = 0;
}

double seconds_since(struct timeval *a, struct timeval *b) {
  double seconds_a = a->tv_sec + a->tv_usec * 1e-6;
  double seconds_b = b->tv_sec + b->tv_usec * 1e-6;
  return seconds_b - seconds_a;
}

bool rate_data_add_event(struct rate_data_t *data) {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  data->total_events++;
  data->last_window_events++;
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
    fprintf(stderr,
            "---SSS--- window since start=%lf, seconds_since_window=%lf "
            "intervals=%d\n",
            seconds_since_start, seconds_since_last_window,
            last_window_intervals);
    return true;
  }
  return false;
}

int rate_data_get_total_events(struct rate_data_t *data) {
  return data->total_events;
}

double rate_data_get_total_rate_per_sec(struct rate_data_t *data) {
  return data->total_rate;
}

double rate_data_get_recent_rate_per_sec(struct rate_data_t *data) {
  return data->last_window_rate;
}
