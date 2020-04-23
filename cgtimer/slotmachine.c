/*
 * machine.c
 *
 * Created: 14/04/2020
 * Author:  Chris Hough
 */ 

#ifndef F_CPU				// if F_CPU was not defined in Project -> Properties
#define F_CPU 1000000UL		// define it now as 1 MHz unsigned long
#endif

#include "slotmachine.h"
#include <stdint.h>
#include <stdlib.h>
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
#define BAR_IMAGES 12
#define BAR_Y_SIZE ( (IMAGE_Y_SIZE + 1) * BAR_IMAGES )

// Start = +50.
// Play  = -1
//
// Win
// ---
// TV,       TV,       TV       = +5   Odds = 1/6 * 1/6 * 1/6    =  1 in 216
// INSECT,   INSECT,   INSECT   = +10  Odds = 1/6 * 1/6 * 1/6    =  1 in 216
// UMBRELLA, UMBRELLA, UMBRELLA = +10  Odds = 1/6 * 1/12 * 1/6   =  1 in 432
// BALL,     BALL,     BALL     = +15  Odds = 1/12 * 1/6 * 1/12  =  1 in 864
// FLAG,     FLAG,     FLAG     = +30  Odds = 1/12 * 1/12 * 1/12 =  1 in 1,728
// 
// Loose
// -----
// GHOST, GHOST, GHOST       = -10     Odds = 1/6 * 1/6 * 1/6 = 1 in 216
// INVADER, INVADER, INVADER = -5      Odds = 1/6 * 1/6 * 1/6 = 1 in 216
//
//TV, TV, GHOST, GHOST, BALL, FLAG, INVADER, INVADER, UMBRELLA, UMBRELLA, INSECT, INSECT.
//TV, TV, GHOST, GHOST, BALL, BALL, FLAG, INVADER, INVADER, UMBRELLA, INSECT, INSECT
//TV, TV, GHOST, GHOST, BALL, FLAG, INVADER, INVADER, UMBRELLA, UMBRELLA, INSECT, INSECT

static const uint8_t g_bar1[] = { TV, BALL, GHOST, INSECT, GHOST, UMBRELLA, INSECT, TV, INVADER, FLAG, UMBRELLA, INVADER };
static const uint8_t g_bar2[] = { GHOST, BALL, GHOST, TV, UMBRELLA, TV, BALL, INVADER, FLAG, INSECT, INVADER, INSECT };
static const uint8_t g_bar3[] = { TV, INSECT, BALL, GHOST, UMBRELLA, INVADER, GHOST, INSECT, TV, INVADER, FLAG, UMBRELLA };

static uint8_t g_bar1_idx = 2;
static uint8_t g_bar2_idx = 3;
static uint8_t g_bar3_idx = 5;

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

static uint8_t g_score = 50;

// function declarations.
uint8_t bar_pos(uint8_t index);
void draw_frame(void);
void draw_bars(void);
uint8_t n_indexes(uint8_t from_idx, uint8_t to_idx);
void roll_bars(uint8_t bar1_idx, uint8_t bar2_idx, uint8_t bar3_idx);
void update_score(uint8_t img1, uint8_t img2, uint8_t img3);
void ghosts_x3(void);
void game_over(void);

void draw_bar_at_position(uint8_t * bar, uint8_t center_pos, uint8_t const * const images);
void draw_image(uint8_t * bar, uint8_t const * const img, uint8_t y);
void create_rshift_image(uint8_t * img, uint8_t const * const  img_src, uint8_t shift_n);
uint8_t prev_index(uint8_t index);
uint8_t next_index(uint8_t index);
uint8_t const * const image_ptr(uint8_t image_n);

// Show the entire slot machine using it's current state.
void slot_show()
{
	oled_blank();
	draw_frame();
	draw_bars();
}

// one play of slot machine.
void slot_action(void)
{
	// To do.
	// generate 3 random numbers with range 0 to 11 to supply to roll_bars.
	// temporary using literals.
	
	uint8_t idx1 = rand() % 11;
	uint8_t idx2 = rand() % 11;
	uint8_t idx3 = rand() % 11;
	
	roll_bars(idx1, idx2, idx3);
	
	g_bar1_idx = idx1;
	g_bar2_idx = idx2;
	g_bar3_idx = idx3;

	uint8_t img1 = g_bar1[g_bar1_idx];
	uint8_t img2 = g_bar2[g_bar2_idx];
	uint8_t img3 = g_bar3[g_bar3_idx];

	update_score(img1, img2, img3);
	
	if (g_score == 0)
	{
		game_over();
	}
}

uint8_t bar_pos(uint8_t index)
{
	return index * (IMAGE_Y_SIZE + 1) + 4;
}

// draw the slot machine frame.
void draw_frame(void)
{
	vgfx_buffer_display_at(&g_frame_left[0], 7, 1);
	vgfx_buffer_display_at(&g_frame_separator[0], 3, 18);
	vgfx_buffer_display_at(&g_frame_separator[0], 3, 31);
	vgfx_buffer_display_at(&g_frame_right[0], 7, 44);
}

// draw the bars using it's current state.
void draw_bars(void)
{
	uint8_t bar1[16] = { 0x00 };
	uint8_t bar2[16] = { 0x00 };
	uint8_t bar3[16] = { 0x00 };

	draw_bar_at_position(&bar1[0], bar_pos(g_bar1_idx), &g_bar1[0]);
	vgfx_buffer_display_at(&bar1[0], 8, 9);

	draw_bar_at_position(&bar2[0], bar_pos(g_bar2_idx), &g_bar2[0]);
	vgfx_buffer_display_at(&bar2[0], 8, 22);

	draw_bar_at_position(&bar3[0], bar_pos(g_bar3_idx), &g_bar3[0]);
	vgfx_buffer_display_at(&bar3[0], 8, 35);
}

uint8_t n_indexes(uint8_t from_idx, uint8_t to_idx)
{
	if (from_idx <= to_idx)
	{
		return to_idx - from_idx;
	}

	return BAR_IMAGES - from_idx + to_idx;
}

// roll the bars and stop them at the given indexes.
void roll_bars(uint8_t bar1_idx, uint8_t bar2_idx, uint8_t bar3_idx)
{
	uint8_t bar[16] = { 0x00 };

	uint8_t pos1 = bar_pos(g_bar1_idx);
	uint8_t pos2 = bar_pos(g_bar2_idx);
	uint8_t pos3 = bar_pos(g_bar3_idx);

	uint16_t steps1 = n_indexes(g_bar1_idx, bar1_idx) * (IMAGE_X_SIZE + 1);
	uint16_t steps2 = n_indexes(g_bar2_idx, bar2_idx) * (IMAGE_X_SIZE + 1);
	uint16_t steps3 = n_indexes(g_bar3_idx, bar3_idx) * (IMAGE_X_SIZE + 1);
	
	steps1 += (BAR_Y_SIZE * 2);
	steps2 += (BAR_Y_SIZE * 2);
	steps3 += (BAR_Y_SIZE * 2);

	uint16_t max = steps1;
	if (steps2 > max)
		max = steps2;
	if (steps3 > max)
		max = steps3;

	for (uint16_t i = 1; i <= max; i++)
	{		
		if (steps1 != 0)	
		{
			pos1 = pos1 == BAR_Y_SIZE-1 ? 0 : pos1 + 1;

			vgfx_buffer_clear(&bar[0], 16);
			draw_bar_at_position(&bar[0], pos1, &g_bar1[0]);
			vgfx_buffer_display_at(&bar[0], 8, 9);		

			steps1 --;
		}

		if (steps2 != 0)
		{
			pos2 = pos2 == BAR_Y_SIZE-1 ? 0 : pos2 + 1;

			vgfx_buffer_clear(&bar[0], 16);
			draw_bar_at_position(&bar[0], pos2, &g_bar2[0]);
			vgfx_buffer_display_at(&bar[0], 8, 22);			

			steps2 --;
		}

		if (steps3 != 0)
		{
			pos3 = pos3 == BAR_Y_SIZE-1 ? 0 : pos3 + 1;

			vgfx_buffer_clear(&bar[0], 16);
			draw_bar_at_position(&bar[0], pos3, &g_bar3[0]);
			vgfx_buffer_display_at(&bar[0], 8, 35);

			steps3 --;
		}

		//_delay_ms(5);
	}
}

// update the score.
void update_score(uint8_t img1, uint8_t img2, uint8_t img3)
{
	if (img1 == img2 && img1 == img3)
	{
		switch (img1)
		{
			case TV:
				g_score += 5;
				break;

			case INSECT:
				g_score += 10;
				break;

			case UMBRELLA:
				g_score += 10;
				break;

			case BALL:
				g_score += 15;
				break;

			case FLAG:
				g_score += 30;
				break;
				
			case INVADER:
				if (g_score <= 5)
				{
					g_score = 0;
				}
				else
				{
					g_score -= 5;					
				}
				break;

			case GHOST:
				if (g_score <= 10)
				{
					g_score = 0;
				}
				else
				{
					g_score -= 10;
				}
				break;
		}
	}
}

void ghosts_x3(void)
{
	
}

void game_over(void)
{
	
}


// 8 = image of 8 pixels in y axis, 
// _ = space pixel in y axis between images.
//
//  8  _   8   _   8   _   8   _   8   _   8   _   8    _
// TV,   BALL,   GHOST,  INSECT, GHOST,  UMBRELLA
// 1-8 _ 10-17 _ 19-26 _ 28-35 _ 37-44 _ 46-53 _ 55-62  _
//       prev     |      next
//
// example center (shown as |) = 21
//

// draws the bar containing 2 or 3 images to the buffer.
void draw_bar_at_position(uint8_t * bar, uint8_t center_pos, uint8_t const * const images)
{
	uint8_t center_img_index = center_pos / (IMAGE_Y_SIZE + 1);
	uint8_t center_img_y = center_img_index * (IMAGE_Y_SIZE + 1) + 1;

	// set center image.
	uint8_t y = IMAGE_Y_SIZE + center_img_y - center_pos;
	draw_image(bar, image_ptr(images[center_img_index]), y);

	// set previous image.
	if (y > 2)
	{
		uint8_t img_index = prev_index(center_img_index);
		uint8_t shift_n = IMAGE_Y_SIZE + 2 - y;	
		
		uint8_t img[IMAGE_X_SIZE];		
		create_rshift_image(&img[0], image_ptr(images[img_index]), shift_n);
		draw_image(bar, &img[0], 1);
	}

	// set next image.
	y += (IMAGE_Y_SIZE + 1);
	if (y <= DISPLAY_Y_SIZE)
	{
		uint8_t img_index = next_index(center_img_index);
		draw_image(bar, image_ptr(images[img_index]), y);
	}
}

// returns the previous bar image index.
uint8_t prev_index(uint8_t index)
{
	return index == 0 ? BAR_IMAGES - 1 : index - 1;
}

// returns the next bar image index.
uint8_t next_index(uint8_t index)
{
	return index == BAR_IMAGES - 1 ? 0 : index + 1;
}

// create a right shifted copy of an image.
void create_rshift_image(uint8_t * img, uint8_t const * const  img_src, uint8_t shift_n)
{
	for (uint8_t i = 0; i != IMAGE_X_SIZE; i++)
	{
		img[i] = img_src[i] >> shift_n;
	}
}

// draws the image to the buffer.
void draw_image(uint8_t * bar, uint8_t const * const img, uint8_t y)
{
	vgfx_buffer_or(bar, IMAGE_X_SIZE, img, IMAGE_X_SIZE, 1, y);
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
