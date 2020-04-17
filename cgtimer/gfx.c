/*
 * gfx.c
 *
 * Created: 2020
 * Author:  Chris Hough
 *
 */ 

#include "gfx.h"
#include "cgoled.h"

#define MAX_ROWS 2

// function declarations.
void set_cy_shiftn(uint8_t y, uint8_t * cy, uint8_t * shift_n);


// displays a single column of 8 pixel rows at the given pixel co-ordinates.
void gfx_pixels_at(uint8_t x, uint8_t y, uint8_t pixels)
{
	uint8_t cy = 1;
	uint8_t shift_n = 0;

	set_cy_shiftn(y, &cy, &shift_n);

	if (shift_n == 0)
	{
		oled_write_pixels_at(x, cy, pixels);
	}
	else
	{
		oled_write_pixels_at(x, cy, pixels << shift_n);
		
		cy++;
		shift_n = 8 - shift_n;
		oled_write_pixels_at(x, cy, pixels >> shift_n);	
	}
}


 // displays an image at the given pixel co-ordinates (parameters x, y).
 // the image is a dynamic number of pixel columns (parameter columns).
 // the image is 8 rows of pixels.
void gfx_image_at(uint8_t x, uint8_t y, uint8_t const * const image, uint8_t columns)
{
	// Note: due to the way the display works, the image may span two rows.
	uint8_t cy = 1;
	uint8_t shift_n = 0;

	set_cy_shiftn(y, &cy, &shift_n);
	oled_set_coordinates(x, cy);

	if (shift_n == 0)
	{
		for (uint8_t n = 0; n != columns; n++)
		{
			oled_write_data(*(image + n));
		}
	}
	else
	{
		for (uint8_t n = 0; n != columns; n++)
		{
			oled_write_data((*(image + n)) << shift_n);
		}
		
		if (cy < MAX_ROWS)
		{
			cy++;
			oled_set_coordinates(x, cy);

			shift_n = 8 - shift_n;
			for (uint8_t n = 0; n != columns; n++)
			{
				oled_write_data((*(image + n)) >> shift_n);
			}			
		}
	}
}


 void set_cy_shiftn(uint8_t y, uint8_t * cy, uint8_t * shift_n)
 {
	uint8_t mod = y % 8;

	if (mod == 0)
	{
		*cy = y / 8;
		*shift_n = 7;
	}
	else
	{
		*cy = y / 8 + 1;
		*shift_n = mod - 1;	
	}
 }
