/*
 * time.h
 *
 * Created: 2020
 * Author: Chris Hough
 */ 

#include <stdint.h>

#ifndef TIME_H_
#define TIME_H_

#endif /* TIME_H_ */

typedef struct 
{
	uint8_t hours;
	uint8_t mins;
	uint8_t secs;
}time;

typedef struct
{
	uint8_t tens;
	uint8_t units;
}time_unit;

time seconds_to_time(uint16_t seconds);
time_unit time_unit_to_chars(uint8_t unit);
