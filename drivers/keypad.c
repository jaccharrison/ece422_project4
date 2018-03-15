#include "keypad.h"
#include <ctype.h>

static unsigned keypad_flags;

/**
 * init_keypad
 * Configures I/O ports and Timer A3 for use with a matrix keypad.
 * Sets the keypad into uppercase alphanumeric mode.
 */
void init_keypad(void)
{
  P9DIR = KEYPAD_OUTPUT_PINS; /* Set output pins (used to poll columns) */
  P9REN = KEYPAD_INPUT_PINS; /* Inputs use pull-up resistors */
  P9OUT = KEYPAD_INPUT_PINS; /* Use pull-up resistors instead of pull-down */

  /* Configure TA3 to poll keypad columns
   * Checks what clock signal is driving ACLK to determine what value to
   * write to a count-control register to get an 12Hz polling frequency. 12Hz
   * is a good tradeoff between polling frequency and acceptable keypad
   * responsiveness. */
  TA3CTL = (TASSEL__ACLK | ID_1 | MC__UP); /* Enable TA3 in up mode */
  if ((CSCTL2 & 0x0700) != 0x0100) /* True if ACLK sourced from VLO */
    TA3CCR0 = KEYPAD_POLLING_INT_VLO;
  else
    TA3CCR0 = KEYPAD_POLLING_INT_LFXT;

  keypad_flags = 0; /* Initialize keypad into default mode: */
  set_keypad_case(UPPERCASE); /* Use uppercase ascii codes */

  TA3CCTL0 |= CCIE; /* Enable TA3 Interrupt - begin polling keypad columns */
}

/* lookup tables for values in keypad matrix */
static int alphanum_keypad_lut[9][4] =
  { {'1', 'A', 'B', 'C'},
    {'2', 'D', 'E', 'F'},
    {'3', 'G', 'H', 'I'},
    {'4', 'J', 'K', 'L'},
    {'5', 'M', 'N', 'O'},
    {'6', 'P', 'Q', 'R'},
    {'7', 'S', 'T', 'U'},
    {'8', 'V', 'W', 'X'},
    {'9', 'Y', 'Z', '.'} };

/**
 * set_keypad_case
 * Switches the ascii values in alphanum_keypad_lut to be either uppercase or
 * lowercase, depending on the argument. If case == 0, the ascii values will be
 * set to uppercase. If case != 0, the ascii values will be set to lowercase.
 */
void set_keypad_case(char upper_lower)
{
  unsigned i, j;

  if (upper_lower) {
    /* Case non-zero: set the ascii values in LUT to be lowercase */
    if (keypad_flags & LOWERCASE)
      return; /* Values are already lowercase */
    keypad_flags |= LOWERCASE; /* Set lowercase flag */
    for (i = 8; i; --i)
      for (j = 3; j; --j)
	    alphanum_keypad_lut[i][j] += 0x20;
  } else {
    /* Case is zero: set the ascii values in LUT to be uppercase */
    if (~(keypad_flags & LOWERCASE))
      return; /* Values are already uppercase */
    keypad_flags &= ~LOWERCASE; /* Clear lowercase flag */
    for (i = 8; i; --i)
      for (j = 3; j; --j)
        alphanum_keypad_lut[i][j] -= 0x20;
  }
  return;
}

/**
 * keypad_poll (numeric-only mode)
 * Drives the output pins associated with the matrix keypad columns low, one by
 * one, and reads the inputs associated with the keypad columns. If an input is
 * low for a given column, the numeric value corresponding to that pin is
 * read into `lastval`. If no inputs are driven lo (no keys are pressed),
 * lastval is returned.
 *
 * In numeric mode, 0 is a valid output; lastval is -3 for invalid or no input
 *
 * Keymap:
 * |   1   |  2  |   3   |
 * |   4   |  5  |   6   |
 * |   7   |  8  |   9   |
 * | BKSPC |  0  | ENTER |
 */
int keypad_poll(void)
{
  static int lastval;
  static int inputval = -3; /* -3 used as placeholder value since 0 is valid */

  char button_pressed = 0; /* Tracks whether a button has been pressed */

  /* Drive column 1 low */
  P9OUT &= ~KEYPAD_COL1;
  switch (P9IN | 0xF0) {
  case 0: break; /* No keys pressed */
  case KEYPAD_ROW1: /* 1 key pressed */
    button_pressed = 1; inputval = 1;
    break;
  case KEYPAD_ROW2: /* 4 key pressed */
    button_pressed = 1; inputval = 4;
    break;
  case KEYPAD_ROW3: /* 7 key pressed */
    button_pressed = 1; inputval = 7;
    break;
  case KEYPAD_ROW4: /* BKSPC key pressed */
    button_pressed = 1; inputval = BACKSPACE;
    break;
  default: break;
  }

  /* Drive column 2 low */
  P9OUT |= KEYPAD_COL1; P9OUT &= ~KEYPAD_COL2;
  switch (P9IN | 0xF0) {
  case 0: break; /* No keys pressed */
  case KEYPAD_ROW1: /* 2 key pressed */
    button_pressed = 1; inputval = 2;
    break;
  case KEYPAD_ROW2: /* 5 key pressed */
    button_pressed = 1; inputval = 5;
    break;
  case KEYPAD_ROW3: /* 8 key pressed */
    button_pressed = 1; inputval = 8;
    break;
  case KEYPAD_ROW4: /* 0 key pressed */
    button_pressed = 1; inputval = 0;
    break;
  default: break;
  }

  /* Drive column 3 low */
  P9OUT |= KEYPAD_COL1; P9OUT &= ~KEYPAD_COL3;
  switch (P9IN | 0xF0) {
  case 0: break; /* No keys pressed */
  case KEYPAD_ROW1: /* 3 key pressed */
    button_pressed = 1; inputval = 3;
    break;
  case KEYPAD_ROW2: /* 6 key pressed */
    button_pressed = 1; inputval = 6;
    break;
  case KEYPAD_ROW3: /* 9 key pressed */
    button_pressed = 1; inputval = 9;
    break;
  case KEYPAD_ROW4: /* RET key pressed */
    button_pressed = 1; inputval = RETURN;
    break;
  default: break;
  }

  /* Decide on return value: lastval if no more buttons pressed, else -3 */
  if ((inputval != -3) && !(button_pressed)) {
    /* Transfer temp val into last submitted val */
    lastval = inputval;
    inputval = -3;
    return lastval;
  } else {
    return -3;
  }
}

/**
 * keypad_poll_alphanum (alphanumeric mode)
 * Polls the matrix keypad the same way described for the numeric mode of this
 * function. In alphanum mode, 0 is an invalid character, and is used as the
 * placeholder value when there is no input.
 *
 * For alphanumeric polling, repeated presses of a button cycle through numbers
 * and characters to allow alphanumeric input. This is intended to work like
 * texting did on ancient cellphones.
 *
 * Keymap:
 * | 1ABC  | 2DEF | 3GHI |
 * | 4JKL  | 5MNO | 6PQR |
 * | 7STU  | 8VWX | 9YZ. |
 * | BKSPC |  0   | RET  |
 */
int keypad_poll_alphanum(void)
{
  static int lastval = 0;
  static int inputval = 0;
  static unsigned rpt = 0; /* tracks repeated presses of the same key */
  static char inc_rpt;

  char button_pressed = 0; /* Tracks whether a button is pressed */

  /* Drive column 1 low */
  P9OUT &= ~KEYPAD_COL1;
  switch (P9IN | 0xF0) {
  case 0: break; /* No keys pressed */
  case KEYPAD_ROW1: /* 1 key pressed */
    button_pressed = 1;
    if (lastval=='1' || toupper(lastval)=='A' || toupper(lastval)=='B' ||
	toupper(lastval)=='C') {
      inputval = alphanum_keypad_lut[0][rpt];
      inc_rpt = 1;
    }
    else { inputval = '1'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW2: /* 4 key pressed */
    button_pressed = 1;
    if (lastval=='4' || toupper(lastval)=='J' || toupper(lastval)=='K' ||
	toupper(lastval)=='L') {
      inputval = alphanum_keypad_lut[1][rpt];
      inc_rpt = 1;
    }
    else { inputval = '4'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW3: /* 7 key pressed */
    button_pressed = 1;
    if (lastval=='7' || toupper(lastval)=='S' || toupper(lastval)=='T' ||
	toupper(lastval)=='U') {
      inputval = alphanum_keypad_lut[1][rpt];
      inc_rpt = 1;
    }
    else { inputval = '7'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW4: /* BKSPC key pressed */
    button_pressed = 1;
    inc_rpt = 0;
    inputval = BACKSPACE;
    break;
  default: break;
  }

  /* Drive column 2 lo */
  P9OUT |= KEYPAD_COL1; P9OUT &= ~KEYPAD_COL2;
  switch (P9IN | 0xF0) {
  case 0: break; /* No keys pressed */
  case KEYPAD_ROW1: /* 2 key pressed */
    button_pressed = 1;
    if (lastval=='2' || toupper(lastval)=='D' || toupper(lastval)=='E' ||
	toupper(lastval)=='F') {
      inputval = alphanum_keypad_lut[1][rpt];
      inc_rpt = 1;
    }
    else { inputval = '2'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW2: /* 5 key pressed */
    button_pressed = 1;
    if (lastval=='5' || toupper(lastval)=='M' || toupper(lastval)=='N' ||
	toupper(lastval)=='O') {
      inputval = alphanum_keypad_lut[4][rpt];
      inc_rpt = 1;
    }
    else { inputval = '5'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW3: /* 8 key pressed */
    button_pressed = 1;
    if (lastval=='8' || toupper(lastval)=='V' || toupper(lastval)=='W' ||
	toupper(lastval)=='X') {
      inputval = alphanum_keypad_lut[7][rpt];
      inc_rpt = 1;
    }
    else { inputval = '8'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW4: /* 0 key pressed */
    button_pressed = 1;
    inputval = '0';
    inc_rpt = 0;
    break;
  default: break;
  }

  /* Drive column 3 low */
  P9OUT |= KEYPAD_COL1; P9OUT &= ~KEYPAD_COL3;
  switch (P9IN | 0xF0) {
  case 0: break; /* No keys pressed */
  case KEYPAD_ROW1: /* 3 key pressed */
    button_pressed = 1;
    if (lastval=='3' || toupper(lastval)=='G' || toupper(lastval)=='H' ||
	toupper(lastval)=='I') {
      inputval = alphanum_keypad_lut[2][rpt];
      inc_rpt = 1;
    }
    else { inputval = '3'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW2: /* 6 key pressed */
    button_pressed = 1;
    if (lastval=='6' || toupper(lastval)=='P' || toupper(lastval)=='Q' ||
	toupper(lastval)=='R') {
      inputval = alphanum_keypad_lut[5][rpt];
      inc_rpt = 1;
    }
    else { inputval = '6'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW3: /* 9 key pressed */
    button_pressed = 1;
    if (lastval=='9' || toupper(lastval)=='Y' || toupper(lastval)=='Z' ||
	toupper(lastval)=='Z') {
      inputval = alphanum_keypad_lut[8][rpt];
      inc_rpt = 1;
    }
    else { inputval = '7'; inc_rpt = 0; }
    break;
  case KEYPAD_ROW4: /* RET key pressed */
    button_pressed = 1;
    inc_rpt = 0;
    inputval = RETURN;
    break;
  default: break;
  }

  /* Decide on return value: lastval if no more buttons pressed, else 0 */
  if ((inputval != 0) && !(button_pressed)) {
    /* Decide whether pressed button was a repeat-press */
    if (inc_rpt)
      rpt = (rpt < 3) ? rpt + 1 : 0; /* Increment rpt, modulo 3 */
    else rpt = 0;

    /* Transfer temp val into last submitted val */
    lastval = inputval;
    inputval = 0;
    return lastval;
  } else {
    return 0;
  }
}

/**
 * TIMER3_A0_ISR
 * Sample Code to be added to the TIMER3_A0_ISR, if one already exists, or
 * used outright. Keep in mind that this ISR assumes:
 * - There is a global variable called pollflag
 * - The timer, which is configured in up mode, is counting up to the refresh
 *   interval for the matrix keypad.
 */
#pragma vector = TIMER3_A0_VECTOR
__interrupt void TIMER3_A0_ISR(void)
 {
   extern int pollflag;
   pollflag = 1; /* Signal to main() that the matrix keypad needs polled */
   return;
 }
