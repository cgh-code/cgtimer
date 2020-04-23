/*
 * gfx.h
 *
 * Created: 2020
 * Author:  Chris Hough
 *
 * Functions to provide additional/easier ways of displaying pixels to those supported by the display controller.
 *
 * The display controller's Y co-ordinate (lets call it CY) is a multiple of 8 pixels.
 * When CY=1, Y=1
 * When CY=2, Y=9
 *
 * To displaying an image at co-ordinates Y=3 requires two 8-bit pixel rows (CY=1 and CY=2).
 * The image has to be split to the two rows.
 *
 * These functions provide that ability.
 * They also write direct to the display.
 *
 *
 * Notes: cgoled source routines only supports the display controller features.
 *        vgfx source routines provide the ability to write to a buffer.
 *
 */ 

#include <stdint.h>

#ifndef GFX_H_
#define GFX_H_

// displays a single column of 8 pixel rows at the given pixel co-ordinates.
void gfx_pixels_at(uint8_t x, uint8_t y, uint8_t pixels);

 // displays an image at the given pixel co-ordinates (parameters x, y).
 // the image is a dynamic number of pixel columns (parameter columns).
 // the image is 8 rows of pixels.
void gfx_image_at(uint8_t x, uint8_t y, uint8_t const * const image, uint8_t columns);

#endif /* GFX_H_ */
