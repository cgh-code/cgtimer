/*
 * gfx.h
 *
 * Created: 2020
 * Author:  Chris Hough
 *
 * Functions that provide additional features to those found in the display controller.
 *
 */ 

#ifndef GFX_H_
#define GFX_H_
#endif /* GFX_H_ */

// displays a character at the given pixel co-ordinates.
// not to be confused with the x co-ordinates of the display (which is multiples of 8).
void gfx_character_at(uint8_t x, uint8_t y, uint8_t const * const character);

// displays a column of pixels at the given co-ordinates.
// one pixel column, 8 pixel rows.
void gfx_pixels_at(uint8_t x, uint8_t y, uint8_t pixels);
