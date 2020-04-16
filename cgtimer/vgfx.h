/*
 * vgfx.h
 *
 * Created: 15/04/2020
 * Author:  Chris Hough
 */ 


#ifndef VGFX_H_
#define VGFX_H_
#endif /* VGFX_H_ */

#include <stdint.h>

// clear the contents of the buffer.
void clear_buffer(uint8_t * buffer, uint8_t size);

// set pixels in buffer.
void or_buffer_pixels(uint8_t * buffer, uint8_t buf_xsize, uint8_t buf_ysize, uint8_t const * const pixels, uint8_t pix_xsize, uint8_t x, uint8_t y);

// display the buffer at the given x co-ordinate.
void display_buffer_at(uint8_t const * const buffer, uint8_t buf_xsize, uint8_t x);
