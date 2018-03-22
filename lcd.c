#include "lcd.h"

#define LCD_4bit_tx(A)							\
  P2OUT &= 0xF0; P2OUT |= 0x0F & A; P2OUT |= LCD_E; P2OUT &= ~LCD_E

#define LCD_wait() TA3CCTL0 |= CCIE; TA3CTL = (TASSEL_2 | MC_1);	\
  LPM1

void LCD_txbyte(char); /* Internal function used to send data to the LCD */

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
void init_lcd(int lines, int font)
{
  TA3CCR0 = 800;

  /* Send a sequence of instructions that forces re-init of the LCD */
  LCD_4bit_tx(LCD_INIT);

  TA3CCR0 = LCD_WAIT2; TA3CCTL0 |= CCIE; /* Wait >= 4.1 ms */
  TA3CTL = (TASSEL__SMCLK | ID_1 | MC__UP);
  LPM1; /* Enter LPM3 until timer completes */

  LCD_4bit_tx(LCD_INIT);
  TA3CCR0 = LCD_WAIT3; TA3CCTL0 |= CCIE; /* Wait for >= 100us */
  TA3CTL = (TASSEL__SMCLK | ID_1 | MC__UP);
  LPM1; /* Enter LPM3 until timer completes */
  LCD_txbyte(LCD_FUNC_SET | LCD_FUNC_8BIT | LCD_SET_4BIT);
  /* Finalize 4-bit interface mode and set font set display lines */
  LCD_txbyte(LCD_FUNC_SET + (lines << 3) + (font << 2));
  /* Set the display mode */
  LCD_txbyte(0x0F);
  /* Clear the display */
  LCD_txbyte(LCD_CLEAR_DISP);
  /* Set entry point to home */
  LCD_txbyte(LCD_RETURN_HOME);
  /* Set cursor to move to the right after a character is written */
  LCD_txbyte(LCD_ENTRY_MODE_SET | LCD_ENTRY_INC);
  LCD_wait();
}

/**
 * LCD_txbyte
 * Internal utility function - the real way that any byte of data gets sent to
 * the LCD. This is only used by other functions in the external LCD API - it
 * is not part of the API.
 */
void LCD_txbyte(char c)
{
  LCD_4bit_tx((c & 0xF0) >> 4);
  LCD_wait();
  LCD_4bit_tx(c & 0x0F);
  LCD_wait();
  return;
}

/**
 * lcd_print_char
 * Prints a single character on the LCD
 */
void lcd_print_char(char c)
{
  P2OUT |= LCD_RS;
  LCD_txbyte(c);
  P2OUT &= ~LCD_RS;
  LCD_wait();
}

/**
 * lcd_print_str
 * Prints a null-terminated string starting at the address pointed to by the
 * argument which contains a maximum of one new line character
 */
void lcd_print_str(char *c)
{
  while (*c) {
    P2OUT |= LCD_RS;
    if (*c == '\n') {
      P2OUT &= ~LCD_RS;
      LCD_wait();
      lcd_set_cur(1, 2); /* Set cursor to second line */
      P2OUT |= LCD_RS;
      c++;
    }
    LCD_txbyte(*(c++));
    P2OUT &= ~LCD_RS;
    LCD_wait();
  }
  return;
}

/**
 * lcd_place_str
 * Places a string at a given location on the LCD
 */
void lcd_place_str(char *c, int x, int y)
{
  lcd_set_cur(x, y);
  lcd_print_str(c);
}

/**
 * lcd_set_cur
 * Sets a new cursor position for writing a string
 * Indexes starting with 1, top line is line 1, left column is column 1
 */
void lcd_set_cur(int x, int y)
{
  LCD_txbyte(0x80 + --x + --y * 0x40);
}

/**
 * lcd_clr_screen
 * Clears the lcd screen
 */
void lcd_clr_screen(void)
{
  LCD_txbyte(LCD_CLEAR_DISP);
  LCD_wait();
  return;
}

/**
 * lcd_return_home
 * Sets the curser position to (1, 1)
 */
void lcd_return_home(void)
{
  lcd_set_cur(1, 1);
}

/**
 * TIMER3_A1_VECTOR
 * Interrupt vector responsible for various timing delays
 */
#pragma vector = TIMER3_A0_VECTOR
__interrupt void TIMER3_A0_ISR(void)
{
  TA3CTL = 0; /* Turn off TA3 */
  TA3R = 0; /* Clear counter */
  TA3CCTL0 = 0; /* Disable interrupts and clear pending interrupts */
  LPM1_EXIT; /* Exit LPM3 */
  return;
}
