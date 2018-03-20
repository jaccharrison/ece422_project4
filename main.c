#include <msp430.h>
#include <cs.h> /* TI Clock system driver */
#include <keypad.h>
#include <lcd.h>

#define LF_CRYSTAL_FREQUENCY_IN_HZ 32768
#define HF_CRYSTAL_FREQUENCY_IN_HZ 0

int pollflag, lcdflag;

/* Function prototypes */
void init_clocks(void);
void init_gpio(void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD; /* Disable the watchdog timer */

  init_gpio();
  init_clocks();
  init_keypad();

  /* Count out 15ms with a timer; on expiration, initialize LCD */
  TA3CCR1 = LCD_WAIT1_ACLK;
  TA3CCTL1 |= CCIE; /* Enable interrupt */
  TA3CTL = (TASSEL__ACLK | ID_1 | MC__UP); /* Enable TA3 in up mode */
  _BIS_SR(GIE | LPM3_bits); /* Enter LPM3 until LCD is ready for config */

  init_lcd(); /* Initializes the LCD into 4-bit mode */


  return 0;
}

/**
 * init_gpio
 * Configures I/O on the MSP430 such that:
 * - Pins in port 2 will be used to control the LCD as shown below:
 *   MSP430: | PIN 6 | PIN 5 | PIN 4 | PIN 3 | PIN 2 | PIN 1 | PIN 0 |
 *   LCD:    |  R/~W |  RS   |   E   |  DB7  |  DB6  |  DB5  |  DB4  |
 * - The lower four bits of port 9 are inputs for matrix keypad rows, and pins
 *   9.4-6 of are outputs for the matrix keypad columns
 * - Pins 3.4 and 3.5 are setup for backchannel UART
 */
void init_gpio(void)
{
  PM5CTL0 &= ~LOCKLPM5; /* Unlock GPIO pins */

  /* Enable the LFXT Pins - select primary peripheral for PJ.4 and PJ.5 */
  PJSEL0 |= (BIT4 | BIT5); PJSEL1 &= ~(BIT4 | BIT5);

  /* I/O for the matrix keypad */
  P9DIR = KEYPAD_OUTPUT_PINS; // Set output pins (used to poll columns)
  P9REN = KEYPAD_INPUT_PINS; // Inputs use pull-up resistors
  P9OUT = KEYPAD_INPUT_PINS; // Use pull-up resistors instead of pull-down

  /* I/O for the LCD display */
  P2DIR = LCD_PINS; /* Sets pins 2.0-2.5 as outputs to control LCD */
  P2SEL0 |= BIT4; P2SEL1 &= ~BIT4; /* primary peripheral for 2.4 - TB0.3 */
  P2IES |= BIT4; /* Sets falling edge as interrupt-trig for P2.4 */

  /* I/O for backchannel UART - select primary peripheral function */
  P3SEL0 |= (BIT4 | BIT5); P3SEL1 &= ~(BIT4 | BIT5);
}

void init_clocks(void)
{
  /* Sets the DCO Frequency to 8MHz */
  CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);

  /* Set LFXT and HFXT frequencies */
  CS_setExternalClockSource(
    LF_CRYSTAL_FREQUENCY_IN_HZ,
    HF_CRYSTAL_FREQUENCY_IN_HZ
  );

  /* Enable LFXT crystal */
  CS_turnOnLFXT(CS_LFXT_DRIVE_0);

  /* Set MCLK to 8MHz - source DCO at 8MHz with divider of 1 */
  CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  /* Set SMCLK to 4MHz - source DCO at 8MHz with divider of 8 */
  CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
  /* Set ACLK to 32.768 kHz - source LFXT with a divider of 1 */
  CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}
