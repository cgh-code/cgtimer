# cgtimer

Atmel Studio C project to display the elapsed time on a graphical OLED display.
For fun I added a slot machine game (in development) accessed by pressing button1.


For use with the ATMega328P microcontroller and an OLED display the use the MC0010 controller.  
The MC0010 controller is similar to the HD44780U controller with additional graphical features. 
See folder for MC0010 datasheet.

By default the OLED display is wired to the ATMega328P MCU using the following pins: -


  OLED                     ATMega328P
 ______                  ____      ____
       |                |    |____|    |
       |                | 1         28 | PC5 <----- Button0 
   DB0 | <--------  PD0 | 2         27 | PC4 <----- Button1
   DB1 | <--------  PD1 | 3         26 | PC3 <----- Button2
   DB2 | <--------  PD2 | 4         25 |
   DB3 | <--------  PD3 | 5         24 |
   DB4 | <--------  PD4 | 6         23 |
       |                | 7         22 |
       |                | 8         21 |
       |                | 9         20 |
       |                | 10        19 |
   DB5 | <--------  PD5 | 11        18 |
   DB6 | <--------  PD6 | 12        17 |
   DB7 | <------->  PD7 | 13        16 | PB2  -----
   EN  | <--------  PB0 | 14        15 | PB1  ---  |
       |                |______________|         | |
       |                                         | |
   RW  | <---------------------------------------  |
   RS  | <----------------------------------------- 
       |
 ______|
 
 
 OLED mappings DB0 to DB7, EN, RW and RS are fully configurable by changing the #define statements in cgoled.h
 
 Buttons use a 1K pull-up resistor on VCC.
 