#include <msp430.h>
#include <cs.h>
#include <lcd.h>

void init_gpio(void);
void init_clocks(void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD; /* Stop watchdog timer */

  init_gpio();
  init_clocks();

  /* Count out 15ms with a timer; on expiration, initialize LCD */
  TA3CCR0 = LCD_WAIT1;
  TA3CCTL0 |= CCIE; /* Enable interrupt */
  TA3CTL = (TASSEL__SMCLK | ID_1 | MC__UP); /* Enable TA3 in up mode */
  _BIS_SR(GIE | LPM1_bits); /* Enter LPM3 until LCD is ready for config */
  init_lcd();

  /* Begin LCD driver demo */
  char *c = "Hello Ben!";

  lcd_print_str(c);

  while(1);

  return 0;
}

/**
 * init_gpio
 * Configures I/O on the MSP430 such that:
 * - Pins in port 2 will be used to control the LCD as shown below:
 *   MSP430: | PIN 6 | PIN 5 | PIN 4 | PIN 3 | PIN 2 | PIN 1 | PIN 0 |
 *   LCD:    |  R/~W |  RS   |   E   |  DB7  |  DB6  |  DB5  |  DB4  |
 * - Pins 3.4 and 3.5 are setup for backchannel UART
 */
void init_gpio(void)
{
  PM5CTL0 &= ~LOCKLPM5; /* Unlock GPIO pins */

  /* I/O for 'advance' button */
  P1DIR = 0; P1REN |= (BIT1 | BIT2); P1OUT |= (BIT1 | BIT2);

  /* I/O for the LCD display */
  P2DIR = LCD_PINS; /* Sets pins 2.0-2.5 as outputs to control LCD */
  P2OUT = 0; /* Clear output on P2 */

  /* I/O for backchannel UART - select primary peripheral function */
  P3SEL0 |= (BIT4 | BIT5); P3SEL1 &= ~(BIT4 | BIT5);
}

void init_clocks(void)
{
  /* Sets the DCO Frequency to 8MHz */
  CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);

  /* Set MCLK to 8MHz - source DCO at 8MHz with divider of 1 */
  CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  /* Set SMCLK to 4MHz - source DCO at 8MHz with divider of 2 */
  CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
  /* Set ACLK to ~9.4kHz - source VLO with a divider of 1 */
  CS_initClockSignal(CS_ACLK, CS_VLOCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

/* Port 1 vector - used to advance through the demo */
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  _BIC_SR(LPM3_exit);
  return;
}
