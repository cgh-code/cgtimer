/*
 * machine.c
 *
 * Created: 14/04/2020
 * Author:  Chris Hough
 */ 

#ifndef F_CPU				// if F_CPU was not defined in Project -> Properties
#define F_CPU 1000000UL		// define it now as 1 MHz unsigned long
#endif

#include "machine.h"
#include <stdint.h>
#include <util/delay.h>
#include "vgfx.h"
#include "cgoled.h"

#define TV 1
#define GHOST 2
#define BALL 3
#define FLAG 4
#define INVADER 5
#define UMBRELLA 6
#define INSECT 7

#define DISPLAY_X_SIZE 50
#define DISPLAY_Y_SIZE 16
#define IMAGE_X_SIZE 8
#define IMAGE_Y_SIZE 8
#define BAR_IMAGES 9
#define BAR_Y_SIZE ( (IMAGE_Y_SIZE + 1) * BAR_IMAGES )

static const uint8_t g_bar1[] = { TV, BALL, GHOST, INSECT, GHOST, UMBRELLA, TV, INVADER, FLAG };
static const uint8_t g_bar2[] = { GHOST, BALL, GHOST, TV, GHOST, TV, GHOST, INVADER, FLAG };
static uint8_t g_bar1_center = 21;
static uint8_t g_bar2_center = 48;

// least significant bit is top.
// images are 8 x 8 pixels.
static const uint8_t g_tv[] = { 0xFC, 0x85, 0x86, 0x84, 0x84, 0x86, 0x85, 0xFC };
static const uint8_t g_ghost[] = { 0xFE, 0xC1, 0x6D, 0x21, 0x21, 0x6D, 0xC1, 0xFE };
static const uint8_t g_ball[] = { 0x28, 0x7C, 0xEE, 0x54, 0x54, 0xEE, 0x7C, 0x28 };
static const uint8_t g_flag[] = { 0x00, 0xFF, 0x11, 0x1F, 0x3E, 0x22, 0x3E, 0x00, };
static const uint8_t g_invader[] = { 0x58, 0xBC, 0x16, 0x3F, 0x3F, 0x16, 0xBC, 0x58 };
static const uint8_t g_umbrella[] = { 0x18, 0x14, 0x1A, 0xFD, 0xFD, 0x9A, 0x54, 0x18 };
static const uint8_t g_insect[] = { 0xC2, 0xE4, 0x39, 0x7E, 0x7E, 0x39, 0xE4, 0xC2 };

// background frame.
static const uint8_t g_frame_left[] = { 0x80, 0x80, 0x80, 0x80, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x80 };
static const uint8_t g_frame_separator[] = { 0x01, 0xFF, 0x01, 0x80, 0xFF, 0x80 };
static const uint8_t g_frame_right[] = { 0x01, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01 };


// function declarations.
void draw_frame(void);
void draw_bars(void);
void roll_bars(void);
void draw_bar_at_position(uint8_t * bar, uint8_t center_pos, uint8_t const * const images);
uint8_t const * const image_ptr(uint8_t image_n);

void slot_show()
{
	oled_blank();
	draw_frame();
	draw_bars();
}

void slot_action(void)
{
	roll_bars();
}

//void slot_snapshot(uint8_t * buffer)
//{
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_left[0], 7, 1, 1);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_left[7], 7, 1, 9);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_separator[0], 3, 18, 1);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_separator[3], 3, 18, 9);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_separator[0], 3, 31, 1);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_separator[3], 3, 31, 9);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_right[0], 7, 44, 1);
	//vgfx_buffer_or(buffer, 50, 2, &g_frame_right[7], 7, 44, 9);
//
	//uint8_t bar[16] = { 0x00 };
	//draw_bar_at_position(&bar[0], g_bar1_center, &g_bar1[0]);
	//
	//uint8_t * ptr = &bar[0];
	//vgfx_buffer_or(buffer, 50, 2, ptr, IMAGE_X_SIZE, 9, 1);
	//ptr += IMAGE_X_SIZE;
	//vgfx_buffer_or(buffer, 50, 2, ptr, IMAGE_X_SIZE, 9, 9);
//}

void draw_frame(void)
{
	vgfx_buffer_display_at(&g_frame_left[0], 7, 1);
	vgfx_buffer_display_at(&g_frame_separator[0], 3, 18);
	vgfx_buffer_display_at(&g_frame_separator[0], 3, 31);
	vgfx_buffer_display_at(&g_frame_right[0], 7, 44);
}

void draw_bars(void)
{
	uint8_t bar1[16] = { 0x00 };
	uint8_t bar2[16] = { 0x00 };

	draw_bar_at_position(&bar1[0], g_bar1_center, &g_bar1[0]);
	vgfx_buffer_display_at(&bar1[0], 8, 9);

	draw_bar_at_position(&bar2[0], g_bar2_center, &g_bar2[0]);
	vgfx_buffer_display_at(&bar2[0], 8, 22);
}

void roll_bars(void)
{
	uint8_t bar[16] = { 0x00 };

	for (uint8_t i = 1; i < 250; i++)
	{		
		if (i <= 200)
		{
			g_bar1_center++;
		}

		if (i >= 50)
		{
			g_bar2_center++;		
		}

		if (g_bar1_center == BAR_Y_SIZE)
		{
			g_bar1_center = 0;			
		}

		if (g_bar2_center == BAR_Y_SIZE)
		{
			g_bar2_center = 0;
		}
		
		vgfx_buffer_clear(&bar[0], 16);
		draw_bar_at_position(&bar[0], g_bar1_center, &g_bar1[0]);
		vgfx_buffer_display_at(&bar[0], 8, 9);

		vgfx_buffer_clear(&bar[0], 16);
		draw_bar_at_position(&bar[0], g_bar2_center, &g_bar2[0]);
		vgfx_buffer_display_at(&bar[0], 8, 22);

		_delay_ms(8);	
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

	vgfx_buffer_or(bar, IMAGE_X_SIZE, ptr, IMAGE_X_SIZE, 1, y);

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
		vgfx_buffer_or(bar, IMAGE_X_SIZE, ptr, IMAGE_X_SIZE, 1, 1);
	}

	// set next image.
	y += (IMAGE_Y_SIZE + 1);
	if (y <= DISPLAY_Y_SIZE)
	{
		uint8_t img_index = center_img_index == BAR_IMAGES - 1 ? 0 : center_img_index + 1;
		ptr = image_ptr(images[img_index]);
		vgfx_buffer_or(bar, IMAGE_X_SIZE, ptr, IMAGE_X_SIZE, 1, y);		
	}
}

// returns pointer to image.
uint8_t const * const image_ptr(uint8_t image_n)
{
	uint8_t const * addr = &g_tv[0];

	switch(image_n)
	{
		case TV:
			addr = &g_tv[0];
			break;

		case GHOST:
			addr = &g_ghost[0];
			break;

		case BALL:
			addr = &g_ball[0];
			break;

		case FLAG:
			addr = &g_flag[0];
			break;

		case INVADER:
			addr = &g_invader[0];
			break;

		case UMBRELLA:
			addr = &g_umbrella[0];
			break;

		case INSECT:
			addr = &g_insect[0];
			break;
	}

	return addr;
}

