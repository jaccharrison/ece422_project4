/**
 * Matrix keypad API
 * Contains functions for the initialization and control of an external 12-key
 * matrix keypad on the MSP430FR6989.
 * Use of the matrix keypad requires:
 * - Pins 9.0-9.6. These are all available in the breakout pins of the
 *   MSP430FR6989 launchpad.
 * - Timer A3 for a polling interrupt. The initialization function in keypad.c
 *   writes the polling interval for this timer into TA3CCR0 and sets the timer
 *   in up mode. Depending on what you're doing, if you're using TA3 for other
 *   things, you may need to use a different CCR or timer mode.
 */
#ifndef KEYPAD_H
#define KEYPAD_H

#include <msp430.h> /* Device header */

/* I/O definitions */
#define KEYPAD_INPUT_PINS 0x0F /* Bits 0-3 */
#define KEYPAD_OUTPUT_PINS 0x70 /* Bits 4-6 */
#define KEYPAD_ROW1 ~0x01
#define KEYPAD_ROW2 ~0x02
#define KEYPAD_ROW3 ~0x04
#define KEYPAD_ROW4 ~0x08
#define KEYPAD_COL1 0x10
#define KEYPAD_COL2 0x20
#define KEYPAD_COL3 0x40

/* Timer CCR definitions for common clock signals - target polling freq 12Hz */
#define KEYPAD_POLLING_INT_VLO 833
#define KEYPAD_POLLING_INT_LFXT 2730

/* Mode switches */
#define UPPERCASE 0x00
#define LOWERCASE 0x01
#define ALPHANUM 0x00
#define NUM_MODE 0x02

/* Special key value definitions */
#define BACKSPACE -1
#define RETURN -2

/* Initialization function: Configures necessary GPIO pins and timers */
void init_keypad(void);

/* set_keypad_case: Edits the ascii values in the LUT used to decode keypad
   input so that the polling function will return uppercase or lowercase
   ascii values, depending on the argument */
void set_keypad_case(char);

/* Polls the keypad and returns the integer value of the entered digit. This
   assumes that the user wants the keypad to be returning only numeric values.
   If alphanumeric values are desired, use the keypad_poll_alphanum function */
int keypad_poll(void);

/* Polls the keypad and returns the ascii value associated with the entered
   digit (or letter). */
int keypad_poll_alphanum(void);

#endif
