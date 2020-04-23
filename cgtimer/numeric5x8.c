/*
 * numeric5x8.c
 *
 * Created: 23/04/2020
 * Author:  Chris Hough
 */ 

#include "numeric5x8.h"

// characters 8x5 (5 columns) pixels.
static const uint8_t g_zero[] = { 0x3E, 0x51, 0x49, 0x45, 0x3E };
static const uint8_t g_one[] = { 0x00, 0x42, 0x7F, 0x40, 0x00 };
static const uint8_t g_two[] = { 0x42, 0x61, 0x51, 0x49, 0x46 };
static const uint8_t g_three[] = { 0x21, 0x41, 0x45, 0x4B, 0x31 };
static const uint8_t g_four[] = { 0x18, 0x14, 0x12, 0x7F, 0x10 };
static const uint8_t g_five[] = { 0x27, 0x45, 0x45, 0x45, 0x39 };
static const uint8_t g_six[] = { 0x3C, 0x4A, 0x49, 0x49, 0x30 };
static const uint8_t g_seven[] = { 0x01, 0x01, 0x71, 0x0D, 0x03 };
static const uint8_t g_eight[] = { 0x36, 0x49, 0x49, 0x49, 0x36 };
static const uint8_t g_nine[] = { 0x06, 0x49, 0x49, 0x29, 0x1E };

// returns pointer to digit.
uint8_t const * const digit5x8_ptr(uint8_t digit)
{
	uint8_t const * addr = &g_zero[0];

	switch(digit)
	{
		case 0:
		addr = &g_zero[0];
		break;

		case 1:
		addr = &g_one[0];
		break;

		case 2:
		addr = &g_two[0];
		break;

		case 3:
		addr = &g_three[0];
		break;

		case 4:
		addr = &g_four[0];
		break;

		case 5:
		addr = &g_five[0];
		break;

		case 6:
		addr = &g_six[0];
		break;

		case 7:
		addr = &g_seven[0];
		break;

		case 8:
		addr = &g_eight[0];
		break;

		case 9:
		addr = &g_nine[0];
		break;
	}

	return addr;
}
