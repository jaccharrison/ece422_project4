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
#define LCD_PINS 0x3F /* Pins 2.0 - 2.6 */
#define LCD_DATA_PINS 0x0F /* Pins 2.0-2.3 */
#define LCD_E (BIT4) /* Pin 4 of LCD control pins */
#define LCD_RS (BIT5) /* Pin 5 of LCD control pins */

/* Wait times for initialization */
#define LCD_WAIT1 60100  /* ACLK cycles to wait for initial 15 ms delay */
#define LCD_WAIT2 16500 /* 4.1 ms second stage delay req'd during init */
#define LCD_WAIT3 400 /* >= 100us third stage delay req'd during init */

/* LCD control bit patterns */
#define LCD_CLEAR_DISP 0x01 /* Clears display */

#define LCD_RETURN_HOME 0x02 /* Returns cursor to home position */

#define LCD_ENTRY_MODE_SET 0x04 /* Sets write mode */
#define LCD_ENTRY_INC 0x02 /* If set, moves cursor to the right after write */
#define LCD_ENTRY_SHIFT 0x01 /* If set, moves display instead of cursor */

#define LCD_DISP_CTL 0x08 /* Configure display control */
#define LCD_DISP_ON 0x04 /* Turns on LCD display */
#define LCD_DISP_CURSOR 0x02 /* Shows LCD cursor */
#define LCD_CURSOR_BLINK 0x01 /* Causes cursor to blink */

#define LCD_FUNC_SET 0x20 /* Function set */
#define LCD_INIT 0x03 /* 4-bit version of LCD_FUNC_SET | LCD_FUNC_8BIT */
#define LCD_FUNC_8BIT 0x10 /* If set, LCD inits into 8-bit mode */
#define LCD_FUNC_2LINE 0x80 /* If set, both LCD lines are enabled */
#define LCD_ALT_FONT 0x40 /* If set, LCD uses alternate font */
#define LCD_SET_4BIT 0x02 /* Control pattern to set 4-bit operation */

void init_lcd(void); /* LCD intialization function */
void lcd_print(char); /* Prints a single character on the LCD */
void lcd_print_str(char*); /* Prints a message on the LCD */
void lcd_clr_screen(void); /* Clears the LCD screen */

#endif
