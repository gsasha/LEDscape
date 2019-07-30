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

RateScheduler::RateScheduler(int times_per_second) {
  gettimeofday(&frame_tv, NULL);
  step_frame_tv.tv_sec = 0;
  step_frame_tv.tv_usec = 1000000 / times_per_second;
}

void RateScheduler::WaitFrame() {
  timeval_add(&frame_tv, &step_frame_tv);

  struct timeval current_time_tv;
  gettimeofday(&current_time_tv, NULL);

  if (timeval_lt(&current_time_tv, &frame_tv)) {
    usleep(timeval_microseconds_until(&current_time_tv, &frame_tv));
  }
}

RateData::RateData(double window_size_seconds)
    : window_size_seconds(window_size_seconds), total_events(0), total_rate(0),
      last_window_events(0), last_window_rate(0) {
  gettimeofday(&initial_time, NULL);
  gettimeofday(&last_window_time, NULL);
}

double seconds_since(struct timeval *a, struct timeval *b) {
  double seconds_a = a->tv_sec + a->tv_usec * 1e-6;
  double seconds_b = b->tv_sec + b->tv_usec * 1e-6;
  return seconds_b - seconds_a;
}

bool RateData::AddEvent() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  total_events++;
  last_window_events++;
  double seconds_since_last_window =
      seconds_since(&last_window_time, &current_time);
  if (seconds_since_last_window >= window_size_seconds) {
    double seconds_since_start = seconds_since(&initial_time, &current_time);
    total_rate = total_events / seconds_since_start;
    int last_window_intervals = last_window_events - 1;
    last_window_rate = last_window_intervals / seconds_since_last_window;

    last_window_time = current_time;
    last_window_events = 0;

    //fprintf(stderr,
    //        "---SSS--- window since start=%lf, seconds_since_window=%lf "
    //        "intervals=%d\n",
    //        seconds_since_start, seconds_since_last_window,
    //        last_window_intervals);
    return true;
  }
  return false;
}

int RateData::GetTotalEvents() const { return total_events; }

double RateData::GetTotalRatePerSec() const { return total_rate; }

double RateData::GetRecentRatePerSec() const { return last_window_rate; }
