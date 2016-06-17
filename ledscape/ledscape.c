/**
 * Userspace interface to the WS281x LED strip driver.
 *
 */
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>

#include "ledscape/ledscape.h"
#include "util.h"

/** GPIO pins used by the LEDscape.
 *
 * The device tree should handle this configuration for us, but it
 * seems horribly broken and won't configure these pins as outputs.
 * So instead we have to repeat them here as well.
 *
 * If these are changed, be sure to check the mappings in
 * ws281x.p!
 *
 * See https://github.com/ehayon/BeagleBone-GPIO/blob/master/src/am335x.h
 * for a complete list of pins.
 *
 * TODO: Find a way to unify this with the defines in the .p file
 */
static const uint8_t gpios0[] = {2,  3,  7,  8,  9,  10, 11, 14,
                                 20, 22, 23, 26, 27, 30, 31};

static const uint8_t gpios1[] = {12, 13, 14, 15, 16, 17, 18, 19, 28, 29};

static const uint8_t gpios2[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 22, 23, 24, 25,
};

static const uint8_t gpios3[] = {14, 15, 16, 17, 19, 21};

#define ARRAY_COUNT(a) ((sizeof(a) / sizeof(*a)))

/** Command structure shared with the PRU.
 *
 * This is mapped into the PRU data RAM and points to the
 * frame buffer in the shared DDR segment.
 *
 * Changing this requires changes in ws281x.p
 */
typedef struct ws281x_command {
  // in the DDR shared with the PRU
  uintptr_t pixels_dma;

  // Length in pixels of the longest LED strip.
  unsigned num_pixels;

  // write 1 to start, 0xFF to abort. will be cleared when started
  volatile unsigned command;

  // will have a non-zero response written when done
  volatile unsigned response;
} __attribute__((__packed__)) ws281x_command_t;

ledscape_pixel_t *ledscape_strip(ledscape_t *const leds, int strip) {
  return leds->frame + strip * leds->leds_per_strip;
}

void ledscape_strip_set_color(ledscape_t *leds, int strip_index,
                              color_channel_order_t color_channel_order,
                              uint8_t *buffer, size_t num_pixels) {
  if (num_pixels > leds->leds_per_strip) {
    num_pixels = leds->leds_per_strip;
  }
  ledscape_pixel_t *strip = ledscape_strip(leds, strip_index);
  for (size_t i = 0; i < num_pixels; i++) {
    ledscape_pixel_set_color(strip, color_channel_order,
                             /* r= */ buffer[0], /* g= */ buffer[1],
                             /* b= */ buffer[2]);
    strip++;
    buffer += 3;
  }
}

void ledscape_set_rgba_data(ledscape_t *leds,
                            color_channel_order_t color_channel_order,
                            uint8_t *buffer, size_t num_leds) {
  if (num_leds > leds->num_leds) {
    num_leds = leds->num_leds;
  }
  ledscape_pixel_t *pixel = leds->frame;
  // TODO(gsasha): unroll this, if rgb is not translated, it's just memcpy.
  for (size_t i = 0; i < num_leds; i++) {
    ledscape_pixel_set_color(pixel, color_channel_order,
                             /* r= */ buffer[0], /* g= */ buffer[1],
                             /* b= */ buffer[2]);
    pixel++;
    buffer += 4;
  }
}

void ledscape_copy_frame_to_pru(ledscape_t *leds) {
  int32_t *ddr = (int32_t *)leds->pru0->ddr;
  
  for (size_t strip_index = 0; strip_index < LEDSCAPE_NUM_STRIPS;
       strip_index++) {
    int32_t *strip =
        (int32_t *)leds->frame + strip_index * leds->leds_per_strip;
    for (size_t pixel = 0; pixel < leds->leds_per_strip; pixel++) {
      ddr[pixel * LEDSCAPE_NUM_STRIPS + strip_index] = strip[pixel];
    }
  }
  //for (int i=0; i<200 ;i++) { ddr[i]=0x0000cc;}
}

/** Initiate the transfer of a frame to the LED strips */
void ledscape_draw(ledscape_t *const leds) {
  ledscape_wait(leds);
  // Now pru is not processing buffers, we can copy them over.
  ledscape_copy_frame_to_pru(leds);

  leds->ws281x_0->pixels_dma = leds->pru0->ddr_addr;
  leds->ws281x_1->pixels_dma = leds->pru0->ddr_addr;

  // Wait for any current command to have been acknowledged
  while (leds->ws281x_0->command || leds->ws281x_1->command) {
    // busy wait.
  }

  // Zero the responses so we can wait for them
  leds->ws281x_0->response = leds->ws281x_1->response = 0;

  // Send the start command
  leds->ws281x_0->command = 1;
  leds->ws281x_1->command = 1;
}

/** Wait for the current frame to finish transfering to the strips.
 * \returns a token indicating the response code.
 */
void ledscape_wait(ledscape_t *const leds) {
  while (1) {
    pru_wait_interrupt();

    //fprintf(stderr, "pru0: (%d,%d), pru1: (%d,%d)\n",
    //        leds->ws281x_0->command, leds->ws281x_0->response,
    //        leds->ws281x_1->command, leds->ws281x_1->response);

    if (leds->ws281x_0->response && leds->ws281x_1->response)
      return;
  }
}

ledscape_t *ledscape_init(unsigned leds_per_strip) {
  return ledscape_init_with_programs(
      leds_per_strip, "pru/bin/ws281x-original-ledscape-pru0.bin",
      "pru/bin/ws281x-original-ledscape-pru1.bin");
}

ledscape_t *ledscape_init_with_programs(unsigned leds_per_strip,
                                        const char *pru0_program_filename,
                                        const char *pru1_program_filename) {
  pru_t *const pru0 = pru_init(0);
  pru_t *const pru1 = pru_init(1);

  const size_t frame_size = leds_per_strip * LEDSCAPE_NUM_STRIPS * 4;

  if (2 * frame_size > pru0->ddr_size) {
    die("Pixel data needs at least 2 * %zu, only %zu in DDR\n", frame_size,
        pru0->ddr_size);
  }

  ledscape_t *const leds = calloc(1, sizeof(*leds));

  ledscape_pixel_t* frame = malloc(frame_size);
  memset(frame, 0, frame_size);

  *leds = (ledscape_t){.pru0 = pru0,
                       .pru1 = pru1,
                       .leds_per_strip = leds_per_strip,
                       .num_leds = leds_per_strip * LEDSCAPE_NUM_STRIPS,
                       .pru0_program_filename = pru0_program_filename,
                       .pru1_program_filename = pru1_program_filename,
                       .frame = frame,
                       .ws281x_0 = pru0->data_ram,
                       .ws281x_1 = pru1->data_ram};

  *(leds->ws281x_0) = *(leds->ws281x_1) = (ws281x_command_t){
      .pixels_dma = 0, // will be set in draw routine
      .command = 0,
      .response = 0,
      .num_pixels = leds->leds_per_strip,
  };

  // Configure all of our output pins.
  for (unsigned i = 0; i < ARRAY_COUNT(gpios0); i++)
    pru_gpio(0, gpios0[i], 1, 0);
  for (unsigned i = 0; i < ARRAY_COUNT(gpios1); i++)
    pru_gpio(1, gpios1[i], 1, 0);
  for (unsigned i = 0; i < ARRAY_COUNT(gpios2); i++)
    pru_gpio(2, gpios2[i], 1, 0);
  for (unsigned i = 0; i < ARRAY_COUNT(gpios3); i++)
    pru_gpio(3, gpios3[i], 1, 0);

  // Initiate the PRU0 program
  pru_exec(pru0, pru0_program_filename);

  // Watch for a done response that indicates a proper startup
  // \todo timeout if it fails
  fprintf(stdout, "String PRU0 with %s... ", pru0_program_filename);
  while (!leds->ws281x_0->response)
    ;
  printf("OK\n");

  // Initiate the PRU1 program
  pru_exec(pru1, pru1_program_filename);

  // Watch for a done response that indicates a proper startup
  // \todo timeout if it fails
  fprintf(stdout, "String PRU1 with %s... ", pru1_program_filename);
  while (!leds->ws281x_1->response)
    ;
  printf("OK\n");

  return leds;
}

const char *
color_channel_order_to_string(color_channel_order_t color_channel_order) {
  switch (color_channel_order) {
  case COLOR_ORDER_RGB:
    return "RGB";
  case COLOR_ORDER_RBG:
    return "RBG";
  case COLOR_ORDER_GRB:
    return "GRB";
  case COLOR_ORDER_GBR:
    return "GBR";
  case COLOR_ORDER_BGR:
    return "BGR";
  case COLOR_ORDER_BRG:
    return "BRG";
  default:
    return "<invalid color_channel_order>";
  }
}

color_channel_order_t color_channel_order_from_string(const char *str) {
  if (strcasecmp(str, "RGB") == 0) {
    return COLOR_ORDER_RGB;
  } else if (strcasecmp(str, "RBG") == 0) {
    return COLOR_ORDER_RBG;
  } else if (strcasecmp(str, "GRB") == 0) {
    return COLOR_ORDER_GRB;
  } else if (strcasecmp(str, "GBR") == 0) {
    return COLOR_ORDER_GBR;
  } else if (strcasecmp(str, "BGR") == 0) {
    return COLOR_ORDER_BGR;
  } else if (strcasecmp(str, "BRG") == 0) {
    return COLOR_ORDER_BRG;
  } else {
    return -1;
  }
}

void ledscape_close(ledscape_t *const leds) {
  // Signal a halt command
  leds->ws281x_0->command = 0xFF;
  leds->ws281x_1->command = 0xFF;
  pru_close(leds->pru0);
  pru_close(leds->pru1);
}
