/*
 * machine.c
 *
 * Created: 14/04/2020
 * Author:  Chris Hough
 */ 

#ifndef F_CPU				// if F_CPU was not defined in Project -> Properties
#define F_CPU 1000000UL		// define it now as 1 MHz unsigned long
#endif

#include <stdint.h>
#include <util/delay.h>
#include "vgfx.h"

#define IMG_N_TV 1
#define IMG_N_GHOST 2
#define IMG_N_BALL 3
#define IMG_N_FLAG 4

#define DISPLAY_X_SIZE 50
#define DISPLAY_Y_SIZE 16
#define IMAGE_X_SIZE 8
#define IMAGE_Y_SIZE 8
#define BAR_IMAGES 7
#define BAR_Y_SIZE ( (IMAGE_Y_SIZE + 1) * BAR_IMAGES )

// least significant bit is top.
// images are 8 x 8 pixels.
static uint8_t img_tv[] = { 0xFC, 0x85, 0x86, 0x84, 0x84, 0x86, 0x85, 0xFC };
static uint8_t img_ghost[] = { 0xFE, 0xC1, 0x6D, 0x21, 0x21, 0x6D, 0xC1, 0xFE };
static uint8_t img_ball[] = { 0x28, 0x7C, 0xEE, 0x54, 0xEE, 0x7C, 0x28, 0x00, };
static uint8_t img_flag[] = { 0x00, 0xFF, 0x11, 0x1F, 0x3E, 0x22, 0x3E, 0x00, };

// background frame.
static uint8_t left_frame[] = { 0x80, 0x80, 0x80, 0x80, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x80 };
static uint8_t separator[] = { 0x01, 0xFF, 0x01, 0x80, 0xFF, 0x80 };
static uint8_t right_frame[] = { 0x01, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01 };

static uint8_t bar1_images[] = { IMG_N_TV, IMG_N_BALL, IMG_N_GHOST, IMG_N_TV, IMG_N_GHOST, IMG_N_TV, IMG_N_FLAG };

uint8_t g_bar1_center_pos = 21;

// declarations.
uint8_t const * const image_ptr(uint8_t image_n);
void draw_bar_at_position(uint8_t * bar, uint8_t center_pos, uint8_t const * const images);

void display_frame(void)
{
	display_buffer_at(&left_frame[0], 7, 1);
	display_buffer_at(&separator[0], 3, 18);
	display_buffer_at(&separator[0], 3, 31);
	display_buffer_at(&right_frame[0], 7, 44);
}

void display_bars(void)
{
	uint8_t bar[16] = { 0x00 };
	draw_bar_at_position(&bar[0], g_bar1_center_pos, &bar1_images[0]);
	display_buffer_at(&bar[0], 8, 9);
}

void roll_bars(void)
{
	uint8_t bar[16] = { 0x00 };

	for (uint8_t i = 1; i < 200; i++)
	{		
		g_bar1_center_pos++;
		if (g_bar1_center_pos > BAR_Y_SIZE)
		{
			g_bar1_center_pos = 1;			
		}
		
		clear_buffer(&bar[0], 16);
		draw_bar_at_position(&bar[0], g_bar1_center_pos, &bar1_images[0]);
		display_buffer_at(&bar[0], 8, 9);
		_delay_ms(40);	
	}
}


// | = 21
//
//  8  _   8   _   8   _   8   _   8   _   8   _   8    _
// 1-8 _ 10-17 _ 19-26 _ 28-35 _ 37-44 _ 46-53 _ 55-62  _
//       prev     |      next
// TV    BALL    GHOST   TV      GHOST   TV      FLAG

// center pos modulus 9 = (21 % 9) = 3
// center pos / 9       = (21 / 9) = 2
//                        (2 * 9) = 18 + 1 = 19.
//
//
// The center line is two rows of pixels.
// Take the top center row of pixels as the center line (call it cline).
// cline has y co-ordinate 8.
//
// When cline points to a space separator row, the next row (y=9) will be the center image.
// When cline points to the last row of an image, previous image will be outside the bounds of the screen.
//
//
void draw_bar_at_position(uint8_t * bar, uint8_t center_pos, uint8_t const * const images)
{
	uint8_t center_img_index = center_pos / (IMAGE_Y_SIZE + 1);
	uint8_t center_img_y = center_img_index * (IMAGE_Y_SIZE + 1) + 1;

	uint8_t const * ptr;
	uint8_t img[IMAGE_X_SIZE];
	
	// set center image.
	ptr = image_ptr(images[center_img_index]);
	uint8_t y = IMAGE_Y_SIZE + center_img_y - center_pos;	
	or_buffer_pixels(bar, IMAGE_X_SIZE, 2, ptr, IMAGE_X_SIZE, 1, y);

	// set previous image.
	if (y > 2)
	{
		uint8_t img_index = center_img_index == 0 ? BAR_IMAGES - 1 : center_img_index - 1;
	
		ptr = image_ptr(images[img_index]);
		uint8_t shift_n = IMAGE_Y_SIZE + 2 - y;
		
		for (uint8_t i = 0; i != IMAGE_X_SIZE; i++)
		{
			img[i] = *ptr >> shift_n;
			ptr++;
		}
		
		ptr = &img[0];	
		or_buffer_pixels(bar, IMAGE_X_SIZE, 2, ptr, IMAGE_X_SIZE, 1, 1);
	}

	// set next image.
	y += (IMAGE_Y_SIZE + 1);
	
	if (y <= DISPLAY_Y_SIZE)
	{
		uint8_t img_index = center_img_index == BAR_IMAGES - 1 ? 0 : center_img_index + 1;
		ptr = image_ptr(images[img_index]);
		or_buffer_pixels(bar, IMAGE_X_SIZE, 2, ptr, IMAGE_X_SIZE, 1, y);		
	}
}

// returns pointer to image.
uint8_t const * const image_ptr(uint8_t image_n)
{
	uint8_t const * addr = &img_tv[0];

	switch(image_n)
	{
		case IMG_N_TV:
			addr = &img_tv[0];
			break;

		case IMG_N_GHOST:
			addr = &img_ghost[0];
			break;

		case IMG_N_BALL:
			addr = &img_ball[0];
			break;

		case IMG_N_FLAG:
			addr = &img_flag[0];
			break;
	}

	return addr;
}

