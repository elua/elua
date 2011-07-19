// Mizar32 LCD character display

#ifndef __DISP_H__
#define __DISP_H__

// See the Ampire datasheet http://home.comet.bg/datasheets/LCD/AC-162B.pdf
// and http://embeddedtutorial.com/2010/01/interfacing-lcd-with-8051/

// I2C bus frequency that the LCD display runs at: 20kHz max
#define DISP_BUS_FREQ 20000

// Pause required after every command byte, and n*delay when sending N
// characters of data (or of commands): 5 milliseconds
// Expressed in CPU clock ticks.
// 5 and 6ms seem not to work when doing
//   mizar32.disp.clear() mizar32.disp.print("Hello world")
// or
//   for i = 1,16 do mizar32.disp.goto(2,i) mizar32.disp.print("X") end
// but 7ms works for both.
#define DISP_DELAY_TICKS (REQ_CPU_FREQ * 7 / 1000)	

// I2C slave addresses for command bytes and data strings
// Command address is followed by a dingle byte giving the command to perform
// Data address is followed by multiple bytes of ASCII data to display
// on the character display at the current cursor location.
#define DISP_CMD  0xF6
#define DISP_DATA 0xF2

// Command bytes

// "Clear display: Write "20H" to DDRAM and set DDRAM address to "00H" from AC"
#define DISP_CMD_CLEAR              1

// "Return Home: Sets DDRAM address to "00H" from AC and return cursor to its
// original position if shifted."
#define DISP_CMD_HOME               2  // Bit 0: don't care

// "Sets DD RAM address in address counter"
#define DISP_CMD_DDADDR           128
// Bits 0-7 are the address:
//   00-0F are the first line
//   40-4F are the second line

#endif
