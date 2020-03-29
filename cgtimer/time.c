/*
 * time.c
 *
 * Created: 2020
 * Author: Chris Hough
 */ 

#include "time.h"

time seconds_to_time(uint16_t seconds)
{
	time t = {0,0,0};
	
	if (seconds >= 3600)
	{
		uint16_t n = seconds / 3600;
		t.hours = (uint8_t)(n);
		seconds -= n * 3600;
	}

	if (seconds >= 60)
	{
		uint16_t n = seconds / 60;
		t.mins = (uint8_t)(n);
		seconds -= n * 60;
	}

	t.secs = (uint8_t)seconds;

	return t;
}

time_unit time_unit_to_chars(uint8_t unit)
{
	time_unit time_chars = {0x30, 0x30};

	if (unit > 0)
	{
		uint8_t tens = unit / 10;
		uint8_t units = unit % 10;

		time_chars.tens += tens;
		time_chars.units += units;
	}

	return time_chars;
 }
