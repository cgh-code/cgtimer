/*
 * machine.c
 *
 * Created: 14/04/2020
 * Author:  Chris Hough
 */ 

#include <stdint.h>
#include <util/delay.h>
#include "cgoled.h"
#include "gfx.h"

#define IMG_N_TV 1
#define IMG_N_SAD = 2
#define IMG_N_BALL = 3
#define IMG_N_FLAG = 4

// least significant (right) bit is top.
// images are 10 x 8 pixels.
static uint8_t img_tv[] = { 0xFC, 0x85, 0x86, 0x84, 0x84, 0x86, 0x85, 0xFC };

static uint8_t img_sad[] = { 0x7E, 0x41, 0xA1, 0x95, 0x90, 0x90, 0x95, 0xA1, 0x41, 0x7E };
static uint8_t img_ball[] = { 0x28, 0x7C, 0xEE, 0x54, 0xEE, 0x7C, 0x28, 0x00, };
static uint8_t img_flag[] = { 0x00, 0xFF, 0x11, 0x1F, 0x3E, 0x22, 0x3E, 0x00, };

// background frame.
static uint8_t left_frame[] = { 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x80,  };
static uint8_t separator[] = { 0x01, 0x80, 0xFF, 0xFF, 0x01, 0x80 };
static uint8_t right_frame[] = { 0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01  };

static uint8_t bar1[16] = { 0x00 };
static uint8_t bar2[16] = { 0x00 };
static uint8_t bar3[16] = { 0x00 };

void display_bar(uint8_t bar_x, uint8_t const * const bar);

void display_frame(void)
{
	uint8_t index_n = 0;
	for (uint8_t x = 1; x != 8; x++)
	{
		oled_write_pixels_at(x, 1, left_frame[index_n++]);
		oled_write_pixels_at(x, 2, left_frame[index_n++]);		
	}

	index_n = 0;
	for (uint8_t x = 18; x != 21; x++)
	{
		oled_write_pixels_at(x, 1, separator[index_n++]);
		oled_write_pixels_at(x, 2, separator[index_n++]);
	}

	index_n = 0;
	for (uint8_t x = 31; x != 34; x++)
	{
		oled_write_pixels_at(x, 1, separator[index_n++]);
		oled_write_pixels_at(x, 2, separator[index_n++]);
	}

	index_n = 0;
	for (uint8_t x = 44; x != 51; x++)
	{
		oled_write_pixels_at(x, 1, right_frame[index_n++]);
		oled_write_pixels_at(x, 2, right_frame[index_n++]);
	}
}

void display_image(void)
{
	//gfx_image_at(9, 5, &img_tv[0], 8);
	//gfx_image_at(22, 5, &img_ball[0], 8);
	//gfx_image_at(35, 5, &img_flag[0], 8);
	
	uint8_t bar[16] = { 0x00 };
		
	// middle
	bar[0] = img_tv[0] << 4;
	bar[1] = img_tv[1] << 4;
	bar[2] = img_tv[2] << 4;
	bar[3] = img_tv[3] << 4;
	bar[4] = img_tv[4] << 4;
	bar[5] = img_tv[5] << 4;
	bar[6] = img_tv[6] << 4;
	bar[7] = img_tv[7] << 4;
	
	bar[8] = img_tv[0] >> 4;
	bar[9] = img_tv[1] >> 4;
	bar[10] = img_tv[2] >> 4;
	bar[11] = img_tv[3] >> 4;
	bar[12] = img_tv[4] >> 4;
	bar[13] = img_tv[5] >> 4;
	bar[14] = img_tv[6] >> 4;
	bar[15] = img_tv[7] >> 4;
	
	// top
	bar[0] |= (img_ball[0] >> 5);
	bar[1] |= (img_ball[1] >> 5);
	bar[2] |= (img_ball[2] >> 5);
	bar[3] |= (img_ball[3] >> 5);
	bar[4] |= (img_ball[4] >> 5);
	bar[5] |= (img_ball[5] >> 5);
	bar[6] |= (img_ball[6] >> 5);
	bar[7] |= (img_ball[7] >> 5);

	// bottom
	bar[8] |= (img_flag[0] << 5);
	bar[9] |= (img_flag[1] << 5);
	bar[10] |= (img_flag[2] << 5);
	bar[11] |= (img_flag[3] << 5);
	bar[12] |= (img_flag[4] << 5);
	bar[13] |= (img_flag[5] << 5);
	bar[14] |= (img_flag[6] << 5);
	bar[15] |= (img_flag[7] << 5);

	display_bar(9, &bar[0]);
	//_delay_ms(500);
}

void display_bar(uint8_t x, uint8_t const * const bar)
{
	uint8_t index_n = 0;
	oled_set_coordinates(x, 1);

	for (uint8_t i = 0; i != 8; i++)
	{
		oled_write_data(bar[index_n++]);
	}
	
	oled_set_coordinates(x, 2);

	for (uint8_t i = 0; i != 8; i++)
	{
		oled_write_data(bar[index_n++]);
	}
}

