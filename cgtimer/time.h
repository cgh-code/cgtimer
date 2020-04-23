/*
 * time.h
 *
 * Created: 2020
 * Author: Chris Hough
 */ 

#include <stdint.h>

#ifndef TIME_H_
#define TIME_H_

// time structure (hrs, mins, seconds)
typedef struct 
{
	uint8_t hours;
	uint8_t mins;
	uint8_t secs;
	
}time_t;

// seconds in base 10 structure (thousands, hundreds, tens, units)
typedef struct
{
	uint8_t ten_thousands;
	uint8_t thousands;
	uint8_t hundreds;
	uint8_t tens;
	uint8_t units;
}secs_base10_t;


// convert a number of seconds to a time structure.
time_t seconds_to_time(uint16_t seconds);

// convert a number of seconds to a base 10 structure.
secs_base10_t seconds_to_base10(uint16_t seconds);

#endif /* TIME_H_ */
