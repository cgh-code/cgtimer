/*
 * vgfx.c
 *
 * Created: 15/04/2020
 * Author:  Chris Hough
 */ 

#include "vgfx.h"
#include "cgoled.h"

// function declarations.
uint8_t buffer_index(uint8_t xsize, uint8_t x, uint8_t y);


// clear the contents of the buffer.
void vgfx_buffer_clear(uint8_t * buffer, uint8_t size)
{
	for (uint8_t i = 0; i != size; i++)
	{
		*buffer = 0x00;
		buffer++;
	}
}

// set pixels in buffer.
void vgfx_buffer_or(uint8_t * buffer, uint8_t buffer_columns, uint8_t const * const pixel_bytes, uint8_t columns, uint8_t x, uint8_t y)
{
	// calculate the buffer y-axis position based on the pixel y co-ordinate. (one based)
	uint8_t buf_y = y / 8;
	uint8_t mod = y % 8;
	if (mod != 0)
	{
		buf_y++;
	}

	// convert the buffer co-ordinates to an index position. (zero based)
	uint8_t * buf_ptr = buffer + buffer_index(buffer_columns, x, buf_y);
	
	// determine the bit shift number.
	uint8_t shift_n = 0;
	if (mod == 0)
	{
		shift_n = 7;
	}
	else
	{
		shift_n = mod - 1;
	}
	
	// set the buffer.
	uint8_t const * pix_ptr = pixel_bytes;

	for (uint8_t i = 0; i != columns; i++)
	{
		if (i != 0)
		{
			buf_ptr++;
			pix_ptr++;
		}

		*buf_ptr |= (*pix_ptr) << shift_n;
	}

	if (shift_n != 0)
	{
		// set the buffer.
		buf_y++;
		shift_n = 8 - shift_n;
		
		if (buf_y <= 2)
		{
			buf_ptr = buffer + buffer_index(buffer_columns, x, buf_y);
			pix_ptr = pixel_bytes;
			
			for (uint8_t i = 0; i != columns; i++)
			{
				if (i != 0)
				{
					buf_ptr++;
					pix_ptr++;
				}

				*buf_ptr |= (*pix_ptr) >> shift_n;
			}
		}
	}
}

// display the buffer at the given x co-ordinate.
void vgfx_buffer_display_at(uint8_t const * const buffer, uint8_t buffer_columns, uint8_t x)
{
	oled_set_coordinates(x, 1);

	uint8_t const * ptr = buffer;
	
	for (uint8_t i = 0; i != buffer_columns; i++)
	{
		oled_write_data(*ptr);
		ptr++;
	}
	
	oled_set_coordinates(x, 2);

	for (uint8_t i = 0; i != buffer_columns; i++)
	{
		oled_write_data(*ptr);
		ptr++;
	}
}


uint8_t buffer_index(uint8_t xsize, uint8_t x, uint8_t y)
{
	uint8_t index = 0;
	
	if (y > 1)
	{
		index += (y-1) * xsize;
	}

	if (x > 1)
	{
		index += x-1;
	}

	return index;
}
