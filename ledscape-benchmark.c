#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define NUM_STRIPS 48
#define LEDS_PER_STRIP 512

typedef int32_t pixels_buffer_t[NUM_STRIPS][LEDS_PER_STRIP];
typedef int32_t pru_buffer_t[LEDS_PER_STRIP][NUM_STRIPS];

void transpose_simple(pixels_buffer_t pixels_buffer, pru_buffer_t pru_buffer) {
  for (int i=0; i<NUM_STRIPS; i++) {
    for (int j=0; j<LEDS_PER_STRIP; j++) {
      pru_buffer[j][i] = pixels_buffer[i][j];
    }
  }
}

void transpose_tiles(pixels_buffer_t pixels_buffer, pru_buffer_t pru_buffer) {
  int tile_size = 4;
  for (int i=0; i<NUM_STRIPS; i+= tile_size) {
    for (int j=0; j<LEDS_PER_STRIP; j+= tile_size) {
      pru_buffer[j][i] = pixels_buffer[i][j];
      pru_buffer[j+1][i] = pixels_buffer[i][j+1];
      pru_buffer[j+2][i] = pixels_buffer[i][j+2];
      pru_buffer[j+3][i] = pixels_buffer[i][j+3];

      pru_buffer[j][i+1] = pixels_buffer[i+1][j];
      pru_buffer[j+1][i+1] = pixels_buffer[i+1][j+1];
      pru_buffer[j+2][i+1] = pixels_buffer[i+1][j+2];
      pru_buffer[j+3][i+1] = pixels_buffer[i+1][j+3];

      pru_buffer[j][i+2] = pixels_buffer[i+2][j];
      pru_buffer[j+1][i+2] = pixels_buffer[i+2][j+1];
      pru_buffer[j+2][i+2] = pixels_buffer[i+2][j+2];
      pru_buffer[j+3][i+2] = pixels_buffer[i+2][j+3];

      pru_buffer[j][i+3] = pixels_buffer[i+3][j];
      pru_buffer[j+1][i+3] = pixels_buffer[i+3][j+1];
      pru_buffer[j+2][i+3] = pixels_buffer[i+3][j+2];
      pru_buffer[j+3][i+3] = pixels_buffer[i+3][j+3];
    }
  }
}

void test_transpose() {
  pixels_buffer_t pixels_buffer;
  pru_buffer_t pru_buffer_expected;
  pru_buffer_t pru_buffer_actual;

  int count = 0;
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < LEDS_PER_STRIP; j++) {
      pixels_buffer[i][j] = count++;
    }
  }

  transpose_simple(pixels_buffer, pru_buffer_expected);
  transpose_tiles(pixels_buffer, pru_buffer_actual);
  for (int i=0; i< NUM_STRIPS; i++) {
    for (int j = 0; j<LEDS_PER_STRIP; j++) {
      if (pru_buffer_expected[j][i] != pru_buffer_actual[j][i]) {
        printf("mismatch at [%d][%d]: %d vs %d\n", j, i,
               pru_buffer_expected[j][i], pru_buffer_actual[j][i]);
        return;
      }
    }
  }
  printf("transpose test ok.\n");
}

struct timer {
  struct timeval start;
  struct timeval end;
};

void timer_start(struct timer* timer) {
  gettimeofday(&timer->start, NULL);
}

void timer_stop(struct timer* timer) {
  gettimeofday(&timer->end, NULL);
}

double timer_diff(struct timer* t) {
  double start_d = t->start.tv_sec + t->start.tv_usec * 1e-6;
  double end_d = t->end.tv_sec + t->end.tv_usec * 1e-6;
  return end_d - start_d;
}

void benchmark_transpose() {
  pixels_buffer_t pixels_buffer;
  pru_buffer_t pru_buffer;

  int count = 0;
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < LEDS_PER_STRIP; j++) {
      pixels_buffer[i][j] = count++;
    }
  }

  int iterations = 10000;

  struct timer simple_timer;
  timer_start(&simple_timer);
  for (int iteration = 0; iteration < iterations; iteration++) {
    transpose_simple(pixels_buffer, pru_buffer);
  }
  timer_stop(&simple_timer);

  printf("Speed of simple transpose %lf ms per iteration\n",
         timer_diff(&simple_timer) / iterations * 1000);

  struct timer tiled_timer;
  timer_start(&tiled_timer);
  for (int iteration = 0; iteration < iterations; iteration++) {
    transpose_tiles(pixels_buffer, pru_buffer);
  }
  timer_stop(&tiled_timer);

  printf("Speed of tiled transpose %lf ms per iteration\n",
         timer_diff(&tiled_timer) / iterations * 1000);

}

int main(int argc, char* argv[]) {
  test_transpose();

  benchmark_transpose();
}

