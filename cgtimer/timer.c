/*
 * timer.c
 *
 * Created: 17/04/2020
 * Author:  Chris Hough
 */ 

#include "timer.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include "time.h"
#include "gfx.h"
#include "cgoled.h"
#include "numeric5x8.h"
//#include "vgfx.h"

// timer modes.
#define MODE_SLEEP 0
#define MODE_IDLE 1
#define MODE_COUNT 2

#define MODE_DISPLAY_TIME 0
#define MODE_DISPLAY_NUMBER 1

// timer1
// 10 bits ... 1024 (1 second)
// 11 bits ...      (1/2 second)
// 1 MHz clock
// counter values is roughly the number of timer ticks in half a second.
#define PRESCALER 11
#define CLOCK_RATE 1000000
#define COUNTER_VALUE (CLOCK_RATE >> PRESCALER)

// characters 8x5 (5 columns) pixels.
static const uint8_t g_space[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t g_comma[] = { 0x80, 0x40 };

// global mode.
static uint8_t g_mode = MODE_IDLE;
static uint8_t g_display = MODE_DISPLAY_TIME;

// globals set by the timer.
static uint16_t g_timer_secs = 0;
static uint16_t g_timer_idle_secs = 0;
static uint8_t g_timer_interval = 0;

// function declarations.
void sleep(void);
void wake(void);
void timer_start(void);
void timer_stop(void);
void timer_clear(void);
void display_time(uint16_t seconds);
void display_time_separator(void);
void display_number(uint16_t seconds);
void clear_time_separator(void);

void timer_config(void)
{
	// output compare register for timer1.
	OCR1A = COUNTER_VALUE;

	// timer counter configuration register for timer1.
	TCCR1A = 0x00;

	// Using CTC (clear timer on compare) mode.
	//
	// At each clock tick it will compare the value of the timer's counter to the OCR1A register.
	// if it matches it's going to clear the counter and provide an interrupt.
	TCCR1B |= (1 << WGM12);

	// Set the prescaler to a value of 1024.
	// Will divide the clock by a value of 1024.
	// The 1MHz clock becomes about 1Khz.
	TCCR1B |= (1 << CS10) | (1 << CS12);

	// Tell the timer I want an interrupt.
	TIMSK1 |= (1 << OCIE1A);

	// globally enable interrupts.
	sei();
}

// Interrupt service routine.
// When timer1 comparison routine matches the value.
ISR(TIMER1_COMPA_vect)
{
	if (g_timer_interval == 0)
	{
		// half a second.
		g_timer_interval = 1;
	}
	else
	{
		// second.
		g_timer_interval = 0;

		if (g_mode == MODE_IDLE)
		{
			g_timer_idle_secs++;
		}
		else if (g_mode == MODE_COUNT)
		{
			g_timer_secs++;
		}
	}
}

void timer_show(void)
{
	oled_blank();
	
	uint16_t timer_secs = g_timer_secs;
	
	if (g_display == MODE_DISPLAY_TIME)
	{
		display_time(timer_secs);
	}
	else
	{
		display_number(timer_secs);	
	}
}


void timer_process(void)
{
	static uint16_t timer_secs = 0;
	static uint8_t timer_interval = 0;
	
	// display the time
	if (g_mode == MODE_COUNT)
	{
		if (timer_secs != g_timer_secs)
		{
			timer_secs = g_timer_secs;
			timer_interval = g_timer_interval;
				
			if (g_display == MODE_DISPLAY_TIME)
				display_time(timer_secs);
			else
				display_number(timer_secs);
				
		}
		else if (timer_interval != g_timer_interval)
		{
			timer_interval = g_timer_interval;
				
			if (g_display == MODE_DISPLAY_TIME)
				clear_time_separator();
		}		
	}	
}

void timer_action(void)
{
	if (g_mode == MODE_SLEEP)
	{
		wake();
	}
	else if (g_mode == MODE_IDLE)
	{
		timer_start();
	}
	else if (g_mode == MODE_COUNT)
	{
		timer_stop();
	}
}

void timer_switch(void)
{
	if (g_display == MODE_DISPLAY_TIME)
	{
		g_display = MODE_DISPLAY_NUMBER;
	}	
	else
	{
		g_display = MODE_DISPLAY_TIME;
	}
	
	timer_show();
}


//void timer_snapshot(uint8_t * buffer)
//{
	//time t = seconds_to_time(g_timer_secs);
	//uint8_t separator = 0x22;
//
	//vgfx_buffer_or(buffer, 50, 2, digit_ptr(t.mins / 10), 5, 13, 6);
	//vgfx_buffer_or(buffer, 50, 2, digit_ptr(t.mins % 10), 5, 19, 6);
	//vgfx_buffer_or(buffer, 50, 2, &separator, 1, 25, 6);
	//vgfx_buffer_or(buffer, 50, 2, digit_ptr(t.secs / 10), 5, 27, 6);
	//vgfx_buffer_or(buffer, 50, 2, digit_ptr(t.secs % 10), 5, 33, 6);
//}

void sleep(void)
{
	// globally disable interrupts.
	cli();

	g_mode = MODE_SLEEP;
	timer_clear();
	oled_power_off();
}

void wake(void)
{
	g_mode = MODE_IDLE;
	timer_clear();
	oled_power_on();

	// globally enable interrupts.
	sei();
}

void timer_start(void)
{
	g_mode = MODE_COUNT;
	timer_clear();
}

void timer_stop(void)
{
	g_mode = MODE_IDLE;
	timer_clear();
	
	if (g_display == MODE_DISPLAY_TIME)
		display_time_separator();
}

void timer_clear(void)
{
	g_timer_interval = 0;
	g_timer_idle_secs = 0;
}

// displays the time vertical centered. (16 pixels in Y axis).
void display_time(uint16_t seconds)
{
	time_t t = seconds_to_time(seconds);

	gfx_image_at(21, 6, digit5x8_ptr(t.mins / 10), 5);
	gfx_image_at(27, 6, digit5x8_ptr(t.mins % 10), 5);

	display_time_separator();

	gfx_image_at(35, 6, digit5x8_ptr(t.secs/ 10), 5);
	gfx_image_at(41, 6, digit5x8_ptr(t.secs % 10), 5);
}

// displays the time vertical centered. (16 pixels in Y axis).
void display_number(uint16_t seconds)
{
	secs_base10_t secs = seconds_to_base10(seconds);

	if (secs.ten_thousands != 0)
		gfx_image_at(14, 6, digit5x8_ptr(secs.ten_thousands), 5);

	if (seconds >= 1000)
	{
		gfx_image_at(20, 6, digit5x8_ptr(secs.thousands), 5);
		gfx_image_at(26, 6, &g_comma[0], 2);
	}

	gfx_image_at(29, 6, digit5x8_ptr(secs.hundreds), 5);
	gfx_image_at(35, 6, digit5x8_ptr(secs.tens), 5);
	gfx_image_at(41, 6, digit5x8_ptr(secs.units), 5);
}

// displays the time separator symbol.
void display_time_separator(void)
{
	gfx_pixels_at(33, 6, 0x22);
}

// clears the time separator symbol.
void clear_time_separator(void)
{
	oled_write_pixels_at(33, 1, 0x00);
	oled_write_pixels_at(33, 2, 0x00);
}


