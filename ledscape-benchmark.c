#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

double time_diff(struct timespec* start, struct timespec* end) {
  double start_d = start->tv_sec + start->tv_nsec * 1e-9;
  double end_d = end->tv_sec + end->tv_nsec * 1e-9;
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

  struct timespec simple_start;
  struct timespec simple_end;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &simple_start);
  for (int iteration = 0; iteration < iterations; iteration++) {
    transpose_simple(pixels_buffer, pru_buffer);
  }
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &simple_end);

  printf("Speed of simple transpose %lf ms per iteration\n",
         time_diff(&simple_start, &simple_end) / iterations * 1000);

  struct timespec tiled_start;
  struct timespec tiled_end;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tiled_start);
  for (int iteration = 0; iteration < iterations; iteration++) {
    transpose_tiles(pixels_buffer, pru_buffer);
  }
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tiled_end);

  printf("Speed of tiled transpose %lf ms per iteration\n",
         time_diff(&tiled_start, &tiled_end) / iterations * 1000);

}

int main(int argc, char* argv[]) {
  test_transpose();

  benchmark_transpose();
}

