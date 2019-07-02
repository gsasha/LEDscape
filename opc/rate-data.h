#ifndef LEDSCAPE_OPC_RATE_DATA_H
#define LEDSCAPE_OPC_RATE_DATA_H

#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

class RateScheduler {
public:
  RateScheduler(int times_per_second);

  void WaitFrame();

private:
  struct timeval frame_tv;
  struct timeval step_frame_tv;
};

class RateData {
  RateData();

  // returns true if new rates have been computed.
  bool AddEvent();
  int GetTotalEvents() const;
  double GetTotalRatePerSec() const;
  double GetRecentRatePerSec() const;

private:
  double window_size_seconds;
  struct timeval initial_time;
  int total_events;
  double total_rate;
  struct timeval last_window_time;
  int last_window_events;
  double last_window_rate;
};

#endif // LEDSCAPE_OPC_RATE_DATA_H
