/** \file
 * LEDscape for the BeagleBone Black.
 *
 * Drives up to 32 ws281x LED strips using the PRU to have no CPU overhead.
 * Allows easy double buffering of frames.
 */

#ifndef _ledscape_h_
#define _ledscape_h_

#include <stdint.h>
#include "pru.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The number of strips supported.
 *
 * Changing this also requires changes in ws281x.p to stride the
 * correct number of bytes per row..
 */
#define LEDSCAPE_NUM_STRIPS 48


/**
 * An LEDscape "pixel" consists of three channels of output and an unused fourth channel. The color mapping of these
 * channels is not defined by the pixel construct, but is specified by color_channel_order_t. Use ledscape_pixel_set_color
 * to assign color values to a pixel.
 */
typedef struct {
	uint8_t a;// was blue
	uint8_t b;// was red
	uint8_t c;// was green
	uint8_t unused;
} __attribute__((__packed__)) ledscape_pixel_t;


/** LEDscape frame buffer is "strip-major".
 *
 * All 32 strips worth of data for each pixel are stored adjacent.
 * This makes it easier to clock out while reading from the DDR
 * in a burst mode.
 */
typedef struct {
	ledscape_pixel_t strip[LEDSCAPE_NUM_STRIPS];
} __attribute__((__packed__)) ledscape_frame_t;

typedef struct ws281x_command ws281x_command_t;

typedef struct {
	ws281x_command_t * ws281x_0;
	ws281x_command_t * ws281x_1;
	pru_t * pru0;
	pru_t * pru1;
	const char* pru0_program_filename;
	const char* pru1_program_filename;
	size_t leds_per_strip;
	size_t num_leds;
        // Frame contains the pixel data in per-strip order.
        ledscape_pixel_t* frame;
} ledscape_t;


typedef enum {
	COLOR_ORDER_RGB,
	COLOR_ORDER_RBG,
	COLOR_ORDER_GRB,
	COLOR_ORDER_GBR,
	COLOR_ORDER_BGR,
	COLOR_ORDER_BRG // Old LEDscape default
} color_channel_order_t;

ledscape_t *ledscape_init(unsigned pixels_per_strip);

ledscape_t *ledscape_init_with_programs(unsigned pixels_per_strip,
                                        const char *pru0_program_filename,
                                        const char *pru1_program_filename);

ledscape_pixel_t *ledscape_strip(ledscape_t *const leds, int strip);

void ledscape_draw(ledscape_t *const leds);

// Set color from a rgb buffer, i.e., triples of uint8_t containing r,g,b
// of each pixel of the strip, continuously.
void ledscape_strip_set_color(
   ledscape_t* leds,
   int strip_index,
   color_channel_order_t color_channel_order,
   uint8_t* buffer,
   size_t num_pixels);

void ledscape_set_raw_data(ledscape_t *leds, uint8_t *buffer,
                           size_t buffer_size);

void ledscape_set_rgba_data(ledscape_t *leds,
                            color_channel_order_t color_channel_order,
                            uint8_t *buffer, size_t num_pixels);

inline void ledscape_pixel_set_color(
	ledscape_pixel_t * const out_pixel,
	color_channel_order_t color_channel_order,
	uint8_t r,
	uint8_t g,
	uint8_t b
) {
	switch (color_channel_order) {
		case COLOR_ORDER_RGB:
			out_pixel->a = r;
			out_pixel->b = g;
			out_pixel->c = b;
		break;

		case COLOR_ORDER_RBG:
			out_pixel->a = r;
			out_pixel->b = b;
			out_pixel->c = g;
		break;

		case COLOR_ORDER_GRB:
			out_pixel->a = g;
			out_pixel->b = r;
			out_pixel->c = b;
		break;

		case COLOR_ORDER_GBR:
			out_pixel->a = g;
			out_pixel->b = b;
			out_pixel->c = r;
		break;

		case COLOR_ORDER_BGR:
			out_pixel->a = b;
			out_pixel->b = g;
			out_pixel->c = r;
		break;

		case COLOR_ORDER_BRG:
			out_pixel->a = b;
			out_pixel->b = r;
			out_pixel->c = g;
		break;
	}
        out_pixel->a = 0xf;
        out_pixel->b = 0xf;
        out_pixel->c = 0xf;
        out_pixel->unused = 0xf;
}

inline void ledscape_set_color(ledscape_t *const leds,
                               color_channel_order_t color_channel_order,
                               uint8_t strip, uint16_t pixel, uint8_t r,
                               uint8_t g, uint8_t b) {
  ledscape_pixel_set_color(ledscape_strip(leds, strip) + pixel,
                           color_channel_order, r, g, b);
}

void ledscape_wait(ledscape_t *const leds);
void ledscape_close(ledscape_t *const leds);

const char *
color_channel_order_to_string(color_channel_order_t color_channel_order);

color_channel_order_t color_channel_order_from_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif
