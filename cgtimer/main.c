/*
 * cgtimer.c
 *
 * Created: 2020
 * Author : Chris Hough
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

// function declarations.
void config_buttons();
void config_timer();

void display_setup();
void display_time(uint16_t seconds);
void display_mode(uint8_t per_second);
bool button1_down();

//void display_hello();
//void display_pixels();

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
	oled_clear();
	oled_incremental_cursor();

	// create the time separator character.
	uint8_t tsep[] = {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
	oled_set_character(1, &tsep[0]);

	// create the up arrow character.
	uint8_t uparrow[] = {0x00, 0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00};
	oled_set_character(2, &uparrow[0]);
}

// displays the time on row 1.
void display_time(uint16_t seconds)
{
	time t = seconds_to_time(seconds);
	time_unit mins_chars = time_unit_to_chars(t.mins);
	time_unit secs_chars = time_unit_to_chars(t.secs);

	oled_write_character(mins_chars.tens, 3, 1);
	oled_write_character(mins_chars.units, 4, 1);
	oled_write_character(0x00, 5, 1);
	oled_write_character(secs_chars.tens, 6, 1);
	oled_write_character(secs_chars.units, 7, 1);
}

// displays the flashing count down symbol.
void display_mode(uint8_t per_second)
{
	if (per_second == 1)
		oled_write_character(0x20, 9, 1);

	else if (per_second == 2)
		oled_write_character(0x01, 9, 1);
}

bool button1_down()
{
	return (PINC & (1 << BTN0));
}

//// Experimenting.
//void display_hello()
//{
	//oled_write_character('H', 1, 1);
	//oled_write_character('e', 2, 1);
	//oled_write_character('l', 3, 1);
	//oled_write_character('l', 4, 1);
	//oled_write_character('o', 5, 1);
	//oled_write_character('0', 6, 1);
	//oled_write_character('1', 7, 1);
	//oled_write_character('2', 8, 1);
	//oled_write_character('3', 9, 1);
	//oled_write_character('4', 10, 1);
	//oled_write_character('5', 11, 1);
	//oled_write_character('6', 12, 1);
	//oled_write_character('7', 13, 1);
	//oled_write_character('8', 14, 1);
	//oled_write_character('9', 15, 1);
//
	//oled_write_character('W', 1, 2);
	//oled_write_character('o', 2, 2);
	//oled_write_character('r', 3, 2);
	//oled_write_character('l', 4, 2);
	//oled_write_character('d', 5, 2);
//
	//_delay_ms(100);
	//oled_write_cmd(CMD_SHIFT_CONTROL | CMD_SHIFT_DISPLAY);
	//_delay_ms(100);
	//oled_write_cmd(CMD_SHIFT_CONTROL | CMD_SHIFT_DISPLAY);
	//_delay_ms(100);
	//oled_write_cmd(CMD_SHIFT_CONTROL | CMD_SHIFT_DISPLAY);
	//_delay_ms(100);
	//oled_write_cmd(CMD_SHIFT_CONTROL | CMD_SHIFT_DISPLAY);
	//_delay_ms(100);
	//oled_write_cmd(CMD_SHIFT_CONTROL | CMD_SHIFT_DISPLAY);
//}
//
//// Experimenting.
//void display_pixels()
//{
	////oled_write_cmd(0x37);
	//oled_write_pixel(1, 1, true);
	////oled_write_pixel(3, 1, true);
	////oled_write_pixel(5, 1, true);
//}
