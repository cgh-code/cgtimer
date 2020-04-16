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
#include "machine.h"

// circuit modes.
#define MODE_SLEEP 0
#define MODE_IDLE 1
#define MODE_COUNT 2

// buttons.
#define BTN0 PC5
#define BTN1 PC4
#define BTN2 PC3

//#define PRESCALER 10

// timer1
// 10 bits ... 1024 (1 second)
// 11 bits ...      (1/2 second)
// 1 MHz clock
#define PRESCALER 11	
#define CLOCK_RATE 1000000

// roughly the number of timer ticks in half a second.
#define COUNTER_VALUE (CLOCK_RATE >> PRESCALER)

// global mode.
uint8_t g_mode = MODE_IDLE;

// globals set by the timer.
uint16_t g_timer_secs = 00;
uint16_t g_timer_idle_secs = 0;
uint8_t g_timer_interval = 0;

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
void config_buttons(void);
void config_timer(void);
void display_setup(void);
void display_time(uint16_t seconds);
void display_time_separator(void);
void clear_time_separator(void);

bool button1_down(void);
bool button2_down(void);
uint8_t const * const digit_ptr(uint8_t digit);

void sleep(void);
void wake(void);
void timer_start(void);
void timer_stop(void);
void timer_clear(void);

//PORTB ^= _BV(PORTB5);

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

int main(void)
{
	// configure the OLED.
	oled_init();
	config_buttons();

	display_setup();
	oled_power_on();
	//display_time(g_timer_secs);
	display_frame();
	display_bars();

	config_timer();

	bool btn1_down_state = false;
	bool btn2_down_state = false;
	uint16_t timer_secs = 0;
	uint8_t timer_interval = 0;

    while (1) 
    {
		// display the time
		if (g_mode == MODE_COUNT)
		{
			if (timer_secs != g_timer_secs)
			{
				timer_secs = g_timer_secs;
				timer_interval = g_timer_interval;
				display_time(timer_secs);
			}
			else if (timer_interval != g_timer_interval)
			{
				timer_interval = g_timer_interval;
				clear_time_separator();
			}
		}
		
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
		
		if (button2_down())
		{
			// wait until button is released before processing another press.
			if (!btn2_down_state)
			{
				btn2_down_state = true;
				roll_bars();
			}
		}
		else
		{
			// button is currently released.
			if (btn2_down_state)
			{
				btn2_down_state = false;
				_delay_ms(20);
			}
		}
	}
}

// alternative main.
// -----------------
// let the interrupts set variables only.  Making the interrupts as quick/short as possible.
// process the interrupt variables each main loop cycle.
//
// Process button has been pressed (global button pressed count != 0). 
//	  a. copy the global count to local variable.
//	  b. decrease the global count.  global count -= local count.  (incase it get increased by an interrupt while processing.)
//    c. Determine the new mode by the local count. 
//       Mode sequence: SLEEP -> IDLE -> COUNT -> IDLE -> COUNT -> IDLE.
//   
//
//
// MODE_COUNT ->
//    When the seconds change (global seconds != local copy of seconds) : -
//    a. save a local copy of the global seconds.  (use local as global may change)
//    b. show the time using the local copy of the seconds.
//
// MODE_IDLE ->
//    When the global idle seconds > sleep seconds.
//    a. switch off the display.
//    b. change mode to MODE_SLEEP.
//
// MODE_SLEEP ->
//	  Do nothing.
//


void config_buttons(void)
{
	// setup buttons for input.
	DDRC &= ~(1 << BTN0);
	DDRC &= ~(1 << BTN1);
	DDRC &= ~(1 << BTN2);
}

void config_timer(void)
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
	oled_clear();
}

void timer_stop(void)
{
	g_mode = MODE_IDLE;
	timer_clear();
	display_time_separator();
}

void timer_clear(void)
{
	g_timer_interval = 0;
	g_timer_idle_secs = 0;
}

// configures the display to: -
// 2 rows of characters.
// characters size of 5 x 7 pixels.
// use 8 bit mode (requires 8 pins for databus).
// auto increment cursor position after writing a character or pixels.
// switches the display to graphics mode which makes the character modes irrelevant until mode is switched.
//
void display_setup(void)
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

// displays the time vertical centered. (16 pixels in Y axis).
void display_time(uint16_t seconds)
{
	time t = seconds_to_time(seconds);

	gfx_image_at(13, 6, digit_ptr(t.mins / 10), 5);
	gfx_image_at(19, 6, digit_ptr(t.mins % 10), 5);

	display_time_separator();

	gfx_image_at(27, 6, digit_ptr(t.secs/ 10), 5);
	gfx_image_at(33, 6, digit_ptr(t.secs % 10), 5);
}

// displays the time separator symbol.
void display_time_separator(void)
{
	gfx_pixels_at(25, 6, 0x22);
}

// clears the time separator symbol.
void clear_time_separator(void)
{
	oled_write_pixels_at(25, 1, 0x00);
	oled_write_pixels_at(25, 2, 0x00);
}

// returns true when button 1 is pressed.
bool button1_down(void)
{
	return !(PINC & (1 << BTN0));
}

// returns true when button 1 is pressed.
bool button2_down(void)
{
	return !(PINC & (1 << BTN1));
}

// returns pointer to digit.
uint8_t const * const digit_ptr(uint8_t digit)
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

