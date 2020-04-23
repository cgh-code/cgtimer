/*
 * vgfx.h
 *
 * Created: 15/04/2020
 * Author:  Chris Hough
 */ 


#include <stdint.h>

#ifndef VGFX_H_
#define VGFX_H_

// clear the contents of the buffer.
void vgfx_buffer_clear(uint8_t * buffer, uint8_t size);

// set pixels in buffer.
void vgfx_buffer_or(uint8_t * buffer, uint8_t buffer_columns, uint8_t const * const pixel_bytes, uint8_t columns, uint8_t x, uint8_t y);

// display the buffer at the given x co-ordinate.
void vgfx_buffer_display_at(uint8_t const * const buffer, uint8_t buffer_columns, uint8_t x);

#endif /* VGFX_H_ */
