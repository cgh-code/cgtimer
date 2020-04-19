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
#include "cgoled.h"
#include "timer.h"
#include "machine.h"
//#include "vgfx.h"   // temporary

// buttons.
#define BTN0 PC5
#define BTN1 PC4
#define BTN2 PC3

#define APP_TIMER 1
#define APP_SLOT_MACHINE 2

static uint8_t g_app = APP_TIMER;

// function declarations.
void config_buttons(void);
bool button1_down(void);
bool button2_down(void);
bool button3_down(void);
void config_display(void);

// button1 -> application action button. 
//            timer application it is start/stop.
//            slot machine application it is start roll.

// button2 -> switch to previous application.
// button3 -> switch to next application.

int main(void)
{
	oled_config();
	config_buttons();
	config_display();
	oled_power_on();

	if (g_app == APP_TIMER)
	{
		timer_show();
	}
	else if (g_app == APP_SLOT_MACHINE)
	{
		slot_show();
	}

	timer_config();

	bool btn1_down_state = false;
	bool btn2_down_state = false;
	bool btn3_down_state = false;

    while (1) 
    {
		if (g_app == APP_TIMER)
		{
			timer_process();
		}
		
		if (button1_down())
		{
			// wait until button is released before processing another press.
			if (!btn1_down_state)
			{
				btn1_down_state = true;

				if (g_app == APP_TIMER)
				{
					timer_action();
				}
				else if (g_app == APP_SLOT_MACHINE)
				{
					slot_action();
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
				
				if (g_app == APP_SLOT_MACHINE)
				{
					g_app = APP_TIMER;
					timer_show();
				}
				else if (g_app == APP_TIMER)
				{
					g_app = APP_SLOT_MACHINE;
					slot_show();
				}			
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

		if (button3_down())
		{
			// wait until button is released before processing another press.
			if (!btn3_down_state)
			{
				btn3_down_state = true;
				
				if (g_app == APP_TIMER)
				{
					timer_switch();
				}
			}
		}
		else
		{
			// button is currently released.
			if (btn3_down_state)
			{
				btn3_down_state = false;
				_delay_ms(20);
			}
		}

	}
}

void config_buttons(void)
{
	// setup buttons for input.
	DDRC &= ~(1 << BTN0);
	DDRC &= ~(1 << BTN1);
	DDRC &= ~(1 << BTN2);
}

// returns true when button 1 is pressed.
bool button1_down(void)
{
	return !(PINC & (1 << BTN0));
}

// returns true when button 2 is pressed.
bool button2_down(void)
{
	return !(PINC & (1 << BTN1));
}

// returns true when button 3 is pressed.
bool button3_down(void)
{
	return !(PINC & (1 << BTN2));
}


// configures the display to: -
// 2 rows of characters.
// characters size of 5 x 7 pixels.
// use 8 bit mode (requires 8 pins for databus).
// auto increment cursor position after writing a character or pixels.
// switches the display to graphics mode which makes the character modes irrelevant until mode is switched.
//
void config_display(void)
{
	oled_write_cmd(CMD_FUNC_CONTROL | CMD_FUNC_8BIT | CMD_FUNC_2LINES);
	oled_cursor_home();
	oled_incremental_cursor();
	oled_graphics_mode();
	oled_clear();
}

// create the up arrow character.
//uint8_t uparrow[] = {0x00, 0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00};
//oled_set_character(2, &uparrow[0]);
//
//PORTB ^= _BV(PORTB5);


				//uint8_t buffer1[100] = { 0x00 };
				//uint8_t buffer2[100] = { 0x00 };
				//uint8_t buffer3[100];
				//
				//slot_snapshot(&buffer1[0]);
				//
				//for (uint8_t i = 1; i != 17; i++)
				//{
				//vgfx_buffer_clear(&buffer3[0], 100);
				//if (i <= 8)
				//{
				//vgfx_buffer_or(&buffer3[0], 50, 2, &buffer1[0], 50, 1, i);
				//vgfx_buffer_or(&buffer3[0], 50, 2, &buffer1[50], 50, 1, i+8);
				//}
				//else
				//{
				//vgfx_buffer_or(&buffer3[0], 50, 2, &buffer1[0], 50, 1, i);
				//}
				//vgfx_buffer_display_at(&buffer3[0], 50, 1);
				//_delay_ms(10);
				//}
				//
				//timer_snapshot(&buffer2[0]);
				//vgfx_buffer_display_at(&buffer2[0], 50, 1);
