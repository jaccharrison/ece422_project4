#include "lcd.h"

/* Holds configuration information */
static unsigned config_flags;

/**
 * init_lcd
 * Initializes the microcontroller for use with an external HD44780U LCD
 * display on port 2.
 *
 * The following code should either be copied into the initialization function
 * below or included in an init_gpio function:
 *   P2DIR = LCD_PINS; // Sets pins 2.0-2.5 as outputs to control LCD
 *
 * 15ms must have passed between LCD power-on and execution of this function.
 */
void init_lcd(void)
{
  /* Initialize TB0 to control the r/w enable LCD pin:
     SMCLK runs at 4MHz - we want out E bit to have a period > 1us and positive
     pulse width >= 450ns. */
  TB0CCTL3 = OUTMOD_3; /* Set-reset out mode on TB0.3 */
  TB0CCR3 = 0x04; /* E set when timer reaches TB0CCR3 */
  TB0CCR0 = 0x08; /* E reset when timer reaches TB0CCR0 */
  TB0CCTL0 |= CCIE; /* Enable TB0.0 interrupt - disables timer */

  P2OUT = 0; /* Initialize all of port 2 out to 0 */

  /* Send a sequence of instructions that forces re-init of the LCD */
  LCD_4bit_tx(LCD_INIT); /* Send re-init instruction */
  /* Wait for another delay (>= 4.1 ms) */
  TA3CCR1 = LCD_WAIT2_ACLK; TA3CCTL1 |= CCIE; /* Load next wait time */
  TA3CTL = (TASSEL__ACLK | ID_1 | MC__UP); /* Start timer */
  _BIS_SR(GIE | LPM3_bits); /* Enter LPM3 until timer completes */
  LCD_4bit_tx(LCD_INIT); /* Send re-init instruction again */
  TA3CTL = (TASSEL__ACLK | ID_1 | MC__UP); /* Wait once more */
  _BIS_SR(GIE | LPM3_bits); /* Enter LPM3 until timer completes */

  LCD_4bit_tx(LCD_INIT);

  /* Watch for interrupt on P2.4 - busy flag */
  LCD_read_mode();
  LCD_busy_wait(); /* Execution will stall - LPM enabled */

  LCD_write_mode();
  LCD_4bit_tx(LCD_SET_4BIT); /* Set LCD into 4-bit mode */
  LCD_read_mode();
  LCD_busy_wait();

  LCD_write_mode();
  LCD_4bit_tx(LCD_SET_4BIT);
  LCD_4bit_tx(0); /* Default font, 1 line */
  LCD_read_mode();
  LCD_busy_wait();

  LCD_write_mode();
  LCD_4bit_tx(0);
  LCD_4bit_tx(1);
  LCD_read_mode();
  LCD_busy_wait();

  LCD_write_mode();
  LCD_4bit_tx(0);
  LCD_4bit_tx(0x06);
  LCD_read_mode();
  LCD_busy_wait();

  LCD_write_mode();
  LCD_4bit_tx(0);
  LCD_4bit_tx(0xE);
  LCD_read_mode();
  LCD_busy_wait();

  LCD_write_mode();
  P2OUT |= LCD_RS;
  LCD_4bit_tx(0x4);
  LCD_4bit_tx(0x8);

  return;
}

/**
 * PORT2_VECTOR
 * Interrupt vector triggered when the busy flag goes low
 */
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
  P2IFG = 0;
  _BIC_SR(LPM1_EXIT);
  return;
}

/**
 * TIMER0_B0_VECTOR
 * Interrupt vector responsible for resetting and disabling TB0 after an enable
 *  signal is sent to the lcd.
 */
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TIMER_B0_ISR(void)
{
  TB0CTL = MC__STOP; /* Stop TB0 */
  TB0R = 0; /* Reset TB0 */
  _BIC_SR(LPM1_EXIT);
}

/**
 * TIMER3_A1_VECTOR
 * Interrupt vector responsible for various timing delays:
 * - Initialization of LCD
 */
#pragma vector = TIMER3_A1_VECTOR
__interrupt void TIMER3_A1_ISR(void)
{
  switch (TA3IV) {
  case 0x00: break; /* No interrupt */
  case 0x02: /* CCR1 interrupt - lcd init timer */
    TA3CTL = 0; /* Turn off TA3 */
    _BIC_SR(LPM3_EXIT); /* Exit LPM3 */
    break;
  default: break;
  }
  return;
}
