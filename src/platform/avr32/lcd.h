// Mizar32 LCD character display

#ifndef __LCD_H__
#define __LCD_H__

// See the Ampire datasheet http://home.comet.bg/datasheets/LCD/AC-162B.pdf
// and http://embeddedtutorial.com/2010/01/interfacing-lcd-with-8051/

// I2C bus frequency that the LCD display runs at.
// It works up to 54kHz, with another window at 67 to 68kHz.
// For reliable operation, we set it to 10% less than the highest "normal" speed.
#define LCD_BUS_FREQ 50000

// 7-bit I2C slave addresses for command bytes and data strings
// Command address is followed by a single byte giving the command to perform
// Data address is followed by multiple bytes of ASCII data to display
// on the character display at the current cursor location.
#define LCD_CMD     (0x7C>>1)    // Send commands
#define LCD_GETPOS  (0x7D>>1)    // Read the cursor position
#define LCD_DATA    (0x7E>>1)    // Send data
#define LCD_BUTTONS (0x7F>>1)    // Read the status of the buttons

// Bits indicating which buttons are held down in the reply to LCD_BUTTONS
#define LCD_BUTTON_SELECT   1
#define LCD_BUTTON_LEFT     2
#define LCD_BUTTON_RIGHT    4
#define LCD_BUTTON_UP       8
#define LCD_BUTTON_DOWN     16

// Command bytes

// Mizar32 LCD driver special: instead of being a NOP, 0 performs a
// reset of the LCD panel
#define LCD_CMD_RESET              0

// "Clear display: Write "20H" to DDRAM and set DDRAM address to "00H" from AC"
#define LCD_CMD_CLEAR              1

// "Return Home: Sets DDRAM address to "00H" from AC and return cursor to its
// original position if shifted."
#define LCD_CMD_HOME               2      // Bit 0: don't care

// "Assign cursor moving direction and enable the shift of entire display"
#define LCD_CMD_ENTRYMODE          4
#define LCD_CMD_ENTRYMODE_SHIFT      1    // 0: move cursor / 1: shift display
#define LCD_CMD_ENTRYMODE_DIRECTION  2    // 0: move/shift left / 1: right
// or, if you prefer...
#define LCD_CMD_ENTRYMODE_MOVE_LEFT   4   // move cursor left when printing
#define LCD_CMD_ENTRYMODE_SHIFT_LEFT  5   // shift display left when printing
#define LCD_CMD_ENTRYMODE_MOVE_RIGHT  6   // move cursor right when printing
#define LCD_CMD_ENTRYMODE_SHIFT_RIGHT 7   // shift display right when printing

// Command 8 uses bits 4,2,1 to set display on/off, underline cursor on/off
// and blinking block on/off.
#define LCD_CMD_DISPLAY_OFF        8
#define LCD_CMD_CURSOR_NONE        12
#define LCD_CMD_CURSOR_BLOCK       13
#define LCD_CMD_CURSOR_LINE        14
#define LCD_CMD_CURSOR_BOTH        15

// "Set cursor moving and display shift control bit, and the direction,
// without changing of DDRAM data".
//
// Actually, this sets no bits at all. It just moves the cursor one place
// left or right or shift the display contents one place left or right.
//
// Command 16 uses bits 8 and 4 to move the cursor one place left or right in
// the character memory and on the display,
// or to shift the displayed characters one place left or right.
// Bit 8 is 0 to move the cursor, 1 to shift the display
// Bit 4 is 0 to move the cursor or to shift the displayed characters left,
//          1 to move the cursor or to shift the displayed characters right
// When shifting the display left, the cursor also moves one place left in the
// physical display, so remains over the same character as before.
#define LCD_CMD_SHIFT              16
#define LCD_CMD_SHIFT_LEFT           0
#define LCD_CMD_SHIFT_RIGHT          4
#define LCD_CMD_SHIFT_CURSOR         0
#define LCD_CMD_SHIFT_DISPLAY        8
// or, if you prefer...
#define LCD_CMD_SHIFT_CURSOR_LEFT   16
#define LCD_CMD_SHIFT_CURSOR_RIGHT  20
#define LCD_CMD_SHIFT_DISPLAY_LEFT  24
#define LCD_CMD_SHIFT_DISPLAY_RIGHT 28

// Command 32 uses bits 16, 8 and 4 to set 
// - interface data length (0 = 4-bit, 16 = 8-bit).  Mizar32 uses 4-bit
// - number of displayed lines (0 = one, 8 = two)
// - display font type (0 = 5x8, 4 = 5x11).
#define LCD_CMD_FUNCTION          32
#define LCD_CMD_FUNCTION_DATABITS_4 0
#define LCD_CMD_FUNCTION_DATABITS_8 16
#define LCD_CMD_FUNCTION_LINES_1    0
#define LCD_CMD_FUNCTION_LINES_2    8
#define LCD_CMD_FUNCTION_FONT_5x8   0
#define LCD_CMD_FUNCTION_FONT_5x11  4

// "Sets CG RAM address in address counter"
// Sets an address in the user-defined character generator RAM.
// The bottom 6 bits define which character to define as code(0-7) * 8.
// Data is then sent to write values 0-31 into the CG RAM, which define a
// character with one byte per row from top to bottom, and bits
// 16, 8, 4, 2, 1 defining the row's pixels left to right.
#define LCD_CMD_CGADDR           64

// "Sets DD RAM address in address counter"
// Bits 0-7 are the address:
//   00-0F are the first line
//   40-4F are the second line
// The location automatically wraps from the end of the first row to the
// start of the second, the end of the second to the start of the first and,
// when printing from right to left, vice versa.
#define LCD_CMD_DDADDR           128

#endif
