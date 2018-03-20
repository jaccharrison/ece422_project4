/**
 * External LCD API
 * Contains functions for the initialization, configuration, and control of an
 * external HD44780U LCD display. Use of the LCD requires:
 * - Pins 2.0-2.7
 */
#ifndef LCD_H
#define LCD_H

#include <msp430.h> /* Device header file */

/* Pin definitions */
#define LCD_PINS 0x3F /* Pins 2.0 - 2.5 */
#define LCD_E 0x10 /* Pin 4 of LCD control pins */
#define LCD_RS 0x20 /* Pin 5 of LCD control pins */
#define LCD_RW 0x40 /* Pin 6 of LCD control pins */
#define LCD_BUSY 0x08 /* D7 - busy flag if LCD_RW == 1 */

/* Initialization and function set instruction definitions */
#define LCD_INIT 0x03 /* Control pattern to force re-initialization of LCD */
#define LCD_SET_4BIT 0x02 /* Control pattern to set 4-bit operation */
#define LCD_WAIT1_ACLK 492  /* ACLK cycles to wait for initial 15 ms delay */
#define LCD_WAIT2_ACLK 135 /* 4.1 ms second stage delay delay */

/* Transmit 4 bits to LCD */
#define LCD_4bit_tx(A) P2OUT&=0xF0; P2OUT|=A; TB0CTL=(TBSSEL_2|MC_2);\
		       TB0CCTL0|=CCIE; _BIS_SR(LPM1_bits|GIE)

/* Continuously toggle E signal - used to read busy flag */
#define LCD_busy_wait() TB0CCTL0&=~CCIE; TB0CTL=(TBSSEL_2|MC_2);\
			_BIS_SR(LPM1_bits|GIE)

/* Sets P2.4 to an input with a pullup and enables its interrupt */
#define LCD_read_mode() P2DIR|=LCD_RW; P2DIR&=~0x80; P2REN|=0x80; P2OUT|=0x80;\
			P2IE=0x80

/* Sets P2.4 to an output and disables its interrupt */
#define LCD_write_mode() P2DIR&=~LCD_RW; P2DIR|=0x80; P2IE&=~0x80

/* Macro that waits for the busy signal to be low */
#define LCD_wait P2OUT|=(LCD_RW); P2DIR&=(~LCD_BUSY);\
		 while(P2IN & LCD_BUSY) { P2OUT|=LCD_E; P2OUT&=~LCD_E; }\
		 P2OUT&=~(LCD_RW); P2DIR|=LCD_BUSY;


#define LCD_enable_rw P2OUT|=LCD_E; P2OUT&=~LCD_E

void init_lcd(void); /* LCD intialization function */

#endif
