/*
 * gfx.c
 *
 * Created: 2020
 * Author:  Chris Hough
*
 * Functions that provide additional features to those found in the display controller.
 *
 */ 

 #include "cgoled.h"

 #define MAX_ROWS 2


 void set_row_and_offset(uint8_t y, uint8_t * row_n, uint8_t * offset_n);

 // displays a character at the given pixel co-ordinates.
 // not to be confused with the x co-ordinates of the display (which is multiples of 8).
 void gfx_character_at(uint8_t x, uint8_t y, uint8_t const * const character)
 {
	// Note: due to the way the display works, the character may span two rows.

	uint8_t row_n = 1;
	uint8_t offset_n = 0;

	set_row_and_offset(y, &row_n, &offset_n);

	oled_set_coordinates(x, row_n);

	if (offset_n == 0)
	{
		for (uint8_t n = 0; n != 5; n++)
		{
			oled_write_data(*(character + n));
		}
		return;
	}

	for (uint8_t n = 0; n != 5; n++)
	{
		oled_write_data((*(character + n)) << offset_n);
	}

	if (row_n >= MAX_ROWS)
		return;

	oled_set_coordinates(x, row_n + 1);

	offset_n = 8 - offset_n;
	for (uint8_t n = 0; n != 5; n++)
	{
		oled_write_data((*(character + n)) >> offset_n);
	}
 }

// displays a column of pixels at the given co-ordinates.
// one pixel column, 8 pixel rows.
void gfx_pixels_at(uint8_t x, uint8_t y, uint8_t pixels)
{
	uint8_t row_n = 1;
	uint8_t offset_n = 0;

	set_row_and_offset(y, &row_n, &offset_n);

	if (offset_n == 0)
	{
		oled_write_pixels_at(x, row_n, pixels);
		return;
	}

	oled_write_pixels_at(x, row_n, pixels << offset_n);
	oled_write_pixels_at(x, row_n + 1, pixels >> (8 - offset_n));
}


 void set_row_and_offset(uint8_t y, uint8_t * row_n, uint8_t * offset_n)
 {
	uint8_t mod = y % 8;

	// calculate the row.
	if (mod == 0)
	{
		*row_n = y / 8;
		*offset_n = 7;
		return;
	}
	
	*row_n = y / 8 + 1;
	*offset_n = mod - 1;
 }
