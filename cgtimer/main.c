/*
 * cgtimer.c
 *
 * Created: 2020
 * Author:  Chris Hough
 */ 

#ifndef F_CPU				// if F_CPU was not defined in Project -> Properties
#define F_CPU 1000000UL		// define it now as 1 MHz unsigned long
#endif

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "time.h"
#include "cgoled.h"
#include "gfx.h"

#define BTN0 PC5

// timer of 10 bits ... 1024
//#define PRESCALER 10
#define PRESCALER 11	// 11 bits 1/2 second

// 1 Mhz clock
#define CLOCK_RATE 1000000

// roughly the number of timer ticks in a second.
#define COUNTER_VALUE (CLOCK_RATE >> PRESCALER)

#define MODE_SLEEP 0
#define MODE_IDLE 1
#define MODE_COUNT 2

// characters 8x5 (5 columns) pixels.
static uint8_t space[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t zero[] = { 0x3E, 0x51, 0x49, 0x45, 0x3E };
static uint8_t one[] = { 0x00, 0x42, 0x7F, 0x40, 0x00 };
static uint8_t two[] = { 0x42, 0x61, 0x51, 0x49, 0x46 };
static uint8_t three[] = { 0x21, 0x41, 0x45, 0x4B, 0x31 };
static uint8_t four[] = { 0x18, 0x14, 0x12, 0x7F, 0x10 };
static uint8_t five[] = { 0x27, 0x45, 0x45, 0x45, 0x39 };
static uint8_t six[] = { 0x3C, 0x4A, 0x49, 0x49, 0x30 };
static uint8_t seven[] = { 0x01, 0x01, 0x71, 0x0D, 0x03 };
static uint8_t eight[] = { 0x36, 0x49, 0x49, 0x49, 0x36 };
static uint8_t nine[] = { 0x06, 0x49, 0x49, 0x29, 0x1E };

// function declarations.
void config_buttons();
void config_timer();

void display_setup();
void display_time(uint16_t seconds);
void display_mode(uint8_t per_second);
bool button1_down();

//void display_hello();
void display_pixels();
uint8_t const * const digit_addr(uint8_t digit);

void sleep();
void wake();
void timer_start();
void timer_stop();


uint8_t g_mode = MODE_IDLE;
uint8_t g_per_second = 0;
uint16_t g_idle_seconds = 0;
uint16_t g_count_seconds = 0;

// Interrupt service routine.
// When timer1 comparison routine matches the value.
//PORTB ^= _BV(PORTB5);
ISR(TIMER1_COMPA_vect)
{
	++g_per_second;
	if (g_per_second > 2)
		g_per_second = 1;

	if (g_per_second == 2)
	{
		if (g_mode == MODE_IDLE)
		{
			// sleep after 10 seconds.
			g_idle_seconds++;
		
			if (g_idle_seconds >= 10)
			{
				sleep();
			}
		}
		else if (g_mode == MODE_COUNT)
		{
			g_count_seconds++;
			display_time(g_count_seconds);
		}
	}

	if (g_mode == MODE_COUNT)
	{
		display_mode(g_per_second);
	}
}

int main(void)
{
	// configure the OLED.
	oled_init();
	config_buttons();

	display_setup();
	display_time(g_count_seconds);
	oled_power_on();
	//display_pixels();

	config_timer();

	bool btn1_down_state = false;

    while (1) 
    {
		if (button1_down())
		{
			// wait until button is released before processing another press.
			if (!btn1_down_state)
			{
				btn1_down_state = true;

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
		}
		else
		{
			// button is currently released.
			if (btn1_down_state)
			{
				btn1_down_state = false;
				_delay_ms(20);
			}
		}
	}
}

void config_buttons()
{
	// setup button for input.
	DDRC &= ~(1 << BTN0);
}

void config_timer()
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

void sleep()
{
	// globally disable interrupts.
	cli();

	g_idle_seconds = 0;
	g_per_second = 0;
	g_mode = MODE_SLEEP;

	oled_power_off();
}

void wake()
{
	g_idle_seconds = 0;
	g_per_second = 0;
	g_mode = MODE_IDLE;

	oled_power_on();

	// globally enable interrupts.
	sei();
}

void timer_start()
{
	g_mode = MODE_COUNT;
}

void timer_stop()
{
	g_idle_seconds = 0;
	g_per_second = 0;
	g_mode = MODE_IDLE;
}

// configures the display to: -
// 2 rows of characters.
// 5 x 7 pixel characters.
// 8 bit mode.
// auto increment cursor position after writing a character.
//
void display_setup()
{
	oled_write_cmd(CMD_FUNC_CONTROL | CMD_FUNC_8BIT | CMD_FUNC_2LINES);
	oled_cursor_home();
	oled_incremental_cursor();
	oled_graphics_mode();
	oled_clear();

	// create the up arrow character.
	//uint8_t uparrow[] = {0x00, 0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00};
	//oled_set_character(2, &uparrow[0]);
}

// displays the time on row 1.
void display_time(uint16_t seconds)
{
	time t = seconds_to_time(seconds);

	gfx_character_at(13, 6, digit_addr(t.mins / 10));
	gfx_character_at(19, 6, digit_addr(t.mins % 10));

	gfx_pixels_at(25, 6, 0x22);

	gfx_character_at(27, 6, digit_addr(t.secs/ 10));
	gfx_character_at(33, 6, digit_addr(t.secs % 10));
}

// displays the flashing count down symbol.
void display_mode(uint8_t per_second)
{
	if (per_second == 1)
	{
		oled_write_pixels_at(25, 1, 0x00);
		oled_write_pixels_at(25, 2, 0x00);
	}

	else if (per_second == 2)
	{
		gfx_pixels_at(25, 6, 0x22);
	}
}

bool button1_down()
{
	return (PINC & (1 << BTN0));
}

uint8_t const * const digit_addr(uint8_t digit)
{
	uint8_t const * addr = &space[0];

	switch(digit)
	{
		case 0:
			addr = &zero[0];
			break;

		case 1:
			addr = &one[0];
			break;

		case 2:
			addr = &two[0];
			break;

		case 3:
			addr = &three[0];
			break;

		case 4:
			addr = &four[0];
			break;

		case 5:
			addr = &five[0];
			break;

		case 6:
			addr = &six[0];
			break;

		case 7:
			addr = &seven[0];
			break;

		case 8:
			addr = &eight[0];
			break;

		case 9:
			addr = &nine[0];
			break;
	}

	return addr;
}


// Experimenting.
void display_pixels()
{
	oled_graphics_mode();
	oled_clear();


	gfx_character_at(1, 3, &zero[0]);
	_delay_ms(15);
	gfx_character_at(1, 3, &space[0]);

	gfx_character_at(7, 5, &zero[0]);
	_delay_ms(15);
	gfx_character_at(7, 3, &space[0]);

	gfx_character_at(13, 7, &zero[0]);
	_delay_ms(15);
	gfx_character_at(13, 3, &space[0]);

	gfx_character_at(19, 9, &zero[0]);
}
