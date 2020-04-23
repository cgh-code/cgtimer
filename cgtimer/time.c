/*
 * time.c
 *
 * Created: 2020
 * Author: Chris Hough
 */ 

#include "time.h"

// convert a number of seconds to a time structure.
time_t seconds_to_time(uint16_t seconds)
{
	time_t t = {0,0,0};
	
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

// convert a number of seconds to a base 10 structure.
secs_base10_t seconds_to_base10(uint16_t seconds)
{
	secs_base10_t secs = { 0, 0, 0, 0, 0 };
	
	if (seconds >= 10000)
	{
		secs.ten_thousands = seconds / 10000;
		seconds -= (secs.ten_thousands * 10000);
	}

	if (seconds >= 1000)
	{
		secs.thousands = seconds / 1000;
		seconds -= (secs.thousands * 1000);
	}
	
	secs.hundreds = seconds / 100;
	seconds -= (secs.hundreds * 100);

	secs.tens = seconds / 10;
	seconds -= (secs.tens * 10);

	secs.units = seconds;
	
	return secs;
}
