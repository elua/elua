
/*
 * Arduino's LiquidCrystal interface: Adapted for XMC4300 (and
 * friends) by Raman Gopalan <ramangopalan@gmail.com>
 * See https://github.com/arduino-libraries/LiquidCrystal
 *
 * LCD Lua module structure adapted from Mizar32's I2C LCD interface.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "lcd.h"
#include "dwt.h"

/********************************************************************************
   Globals
 *******************************************************************************/

lcd_t xmc_lcd;

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

/********************************************************************************
   INITIALIZATION FUNCTIONS
 *******************************************************************************/

void lcd_init_4bits_mode (lcd_t *l) {
  lcd_init(l,
           1,
           LCD_RS_PIN,
           NULL,
           255,
           LCD_ENABLE_PIN,
           LCD_DATA4_PIN,
           LCD_DATA5_PIN,
           LCD_DATA6_PIN,
           LCD_DATA7_PIN,
           NULL, 0,
           NULL, 0,
           NULL, 0,
           NULL, 0);
}

void lcd_init (lcd_t *l,
               uint8_t four_bit_mode,
               XMC_GPIO_PORT_t *const rs_port,
               uint8_t rs_pin,
               XMC_GPIO_PORT_t *const rw_port,
               uint8_t rw_pin,
               XMC_GPIO_PORT_t *const enable_port,
               uint8_t enable_pin,
               XMC_GPIO_PORT_t *const d0_port,
               uint8_t d0_pin,
               XMC_GPIO_PORT_t *const d1_port,
               uint8_t d1_pin,
               XMC_GPIO_PORT_t *const d2_port,
               uint8_t d2_pin,
               XMC_GPIO_PORT_t *const d3_port,
               uint8_t d3_pin,
               XMC_GPIO_PORT_t *const d4_port,
               uint8_t d4_pin,
               XMC_GPIO_PORT_t *const d5_port,
               uint8_t d5_pin,
               XMC_GPIO_PORT_t *const d6_port,
               uint8_t d6_pin,
               XMC_GPIO_PORT_t *const d7_port,
               uint8_t d7_pin) {
  l->rs_port       = rs_port;
  l->rs_pin        = rs_pin;
  l->rw_port       = rw_port;
  l->rw_pin        = rw_pin;
  l->enable_port   = enable_port;
  l->enable_pin    = enable_pin;
  l->data_pins[0]  = d0_pin;
  l->data_pins[1]  = d1_pin;
  l->data_pins[2]  = d2_pin;
  l->data_pins[3]  = d3_pin;
  l->data_pins[4]  = d4_pin;
  l->data_pins[5]  = d5_pin;
  l->data_pins[6]  = d6_pin;
  l->data_pins[7]  = d7_pin;
  l->data_ports[0] = d0_port;
  l->data_ports[1] = d1_port;
  l->data_ports[2] = d2_port;
  l->data_ports[3] = d3_port;
  l->data_ports[4] = d4_port;
  l->data_ports[5] = d5_port;
  l->data_ports[6] = d6_port;
  l->data_ports[7] = d7_port;

  if (four_bit_mode)
    l->display_function = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else 
    l->display_function = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

  lcd_begin(l, LCD_COLS, LCD_ROWS, LCD_5x8DOTS);
}

void lcd_begin (lcd_t *l, uint8_t cols, uint8_t lines, uint8_t dotsize) {
  int i;

  if (lines > 1) {
    l->display_function |= LCD_2LINE;
  }
  l->num_lines = lines;

  lcd_set_row_offsets(l, 0x00, 0x40, 0x00 + cols, 0x40 + cols);

  /* For some 1 line displays you can select a 10 pixel high font */
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    l->display_function |= LCD_5x10DOTS;
  }

  XMC_GPIO_SetMode(l->rs_port, l->rs_pin, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  if (l->rw_pin != 255) {
    XMC_GPIO_SetMode(l->rw_port, l->rw_pin, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
  }
  XMC_GPIO_SetMode(l->enable_port, l->enable_pin, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
  
  /* Do these once, instead of every time a character is drawn for speed reasons. */
  for (i = 0; i < ((l->display_function & LCD_8BITMODE) ? 8U : 4U); ++i) {
    XMC_GPIO_SetMode(l->data_ports[i], l->data_pins[i], XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  dwt_delay(50000);
  // Now we pull both RS and R/W low to begin commands
  XMC_GPIO_SetOutputLow(l->rs_port, l->rs_pin);
  XMC_GPIO_SetOutputLow(l->enable_port, l->enable_pin);
  if (l->rw_pin != 255) {
    XMC_GPIO_SetOutputLow(l->rw_port, l->rw_pin);
  }
  
  //put the LCD into 4 bit or 8 bit mode
  if (!(l->display_function & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    lcd_write4bits(l, 0x03);
    dwt_delay(4500); // wait min 4.1ms

    // second try
    lcd_write4bits(l, 0x03);
    dwt_delay(4500); // wait min 4.1ms
    
    // third go!
    lcd_write4bits(l, 0x03);
    dwt_delay(150);

    // finally, set to 4-bit interface
    lcd_write4bits(l, 0x02);
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    lcd_command(l, LCD_FUNCTIONSET | l->display_function);
    dwt_delay(4500);  // wait more than 4.1ms

    // second try
    lcd_command(l, LCD_FUNCTIONSET | l->display_function);
    dwt_delay(150);

    // third go
    lcd_command(l, LCD_FUNCTIONSET | l->display_function);
  }

  // finally, set # lines, font size, etc.
  lcd_command(l, LCD_FUNCTIONSET | l->display_function);

  // turn the display on with no cursor or blinking default
  l->display_control = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  lcd_display(l);

  // clear it off
  lcd_clear(l);

  // Initialize to default text direction (for romance languages)
  l->display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  lcd_command(l, LCD_ENTRYMODESET | l->display_mode);

}

void lcd_set_row_offsets (lcd_t *l, int row0, int row1, int row2, int row3) {
  l->row_offsets[0] = row0;
  l->row_offsets[1] = row1;
  l->row_offsets[2] = row2;
  l->row_offsets[3] = row3;
}

/********************************************************************************
   USER FUNCTIONS
 *******************************************************************************/

void lcd_clear (lcd_t *l) {
  lcd_command(l, LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  dwt_delay(2000);  // this command takes a long time!
}

void lcd_home (lcd_t *l) {
  lcd_command(l, LCD_RETURNHOME);  // set cursor position to zero
  dwt_delay(2000);  // this command takes a long time!
}

void lcd_set_cursor (lcd_t *l, uint8_t col, uint8_t row) {
  const size_t max_lines = sizeof(l->row_offsets) / sizeof(*l->row_offsets);
  if (row >= max_lines) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if (row >= l->num_lines) {
    row = l->num_lines - 1;    // we count rows starting w/0
  }
  
  lcd_command(l, LCD_SETDDRAMADDR | (col + l->row_offsets[row]));
}

// Turn the display on/off (quickly)
void lcd_no_display (lcd_t *l) {
  l->display_control &= ~LCD_DISPLAYON;
  lcd_command(l, LCD_DISPLAYCONTROL | l->display_control);
}

void lcd_display (lcd_t *l) {
  l->display_control |= LCD_DISPLAYON;
  lcd_command(l, LCD_DISPLAYCONTROL | l->display_control);
}

// Turns the underline cursor on/off
void lcd_no_cursor (lcd_t *l) {
  l->display_control &= ~LCD_CURSORON;
  lcd_command(l, LCD_DISPLAYCONTROL | l->display_control);
}

void lcd_cursor (lcd_t *l) {
  l->display_control |= LCD_CURSORON;
  lcd_command(l, LCD_DISPLAYCONTROL | l->display_control);
}

// Turn on and off the blinking cursor
void lcd_no_blink (lcd_t *l) {
  l->display_control &= ~LCD_BLINKON;
  lcd_command(l, LCD_DISPLAYCONTROL | l->display_control);
}

void lcd_blink (lcd_t *l) {
  l->display_control |= LCD_BLINKON;
  lcd_command(l, LCD_DISPLAYCONTROL | l->display_control);
}

// These commands scroll the display without changing the RAM
void lcd_scroll_display_left (lcd_t *l) {
  lcd_command(l, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void lcd_scroll_display_right (lcd_t *l) {
  lcd_command(l, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void lcd_left_to_right (lcd_t *l) {
  l->display_mode |= LCD_ENTRYLEFT;
  lcd_command(l, LCD_ENTRYMODESET | l->display_mode);
}

// This is for text that flows Right to Left
void lcd_right_to_left (lcd_t *l) {
  l->display_mode &= ~LCD_ENTRYLEFT;
  lcd_command(l, LCD_ENTRYMODESET | l->display_mode);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll (lcd_t *l) {
  l->display_mode |= LCD_ENTRYSHIFTINCREMENT;
  lcd_command(l, LCD_ENTRYMODESET | l->display_mode);
}

// This will 'left justify' text from the cursor
void lcd_no_autoscroll (lcd_t *l) {
  l->display_mode &= ~LCD_ENTRYSHIFTINCREMENT;
  lcd_command(l, LCD_ENTRYMODESET | l->display_mode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_create_char (lcd_t *l, uint8_t location, uint8_t charmap[]) {
  uint32_t i;
  location &= 0x7; // we only have 8 locations 0-7
  lcd_command(l, LCD_SETCGRAMADDR | (location << 3));
  for (i = 0; i < 8; i++) {
    lcd_write(l, charmap[i]);
  }
}

void lcd_echo (lcd_t *l, uint8_t *s, uint32_t len) {
  uint32_t i;

  for (i = 0; i < len; i++)
    lcd_write(l, s[i]);
}

/********************************************************************************
   MID LEVEL FUNCTIONS (FOR SENDING DATA/COMMANDS)
 *******************************************************************************/

inline void lcd_command (lcd_t *l, uint8_t value) {
  lcd_send(l, value, 0U);
}

inline size_t lcd_write (lcd_t *l, uint8_t value) {
  lcd_send(l, value, 1U);
  return 1; // assume success
}

/********************************************************************************
   LOW LEVEL DATA PUSHING FUNCTIONS
 *******************************************************************************/

/* Write either command or data, with automatic 4/8-bit selection */
void lcd_send (lcd_t *l, uint8_t value, uint8_t mode) {
  if (mode)
    XMC_GPIO_SetOutputHigh(l->rs_port, l->rs_pin);
  else
    XMC_GPIO_SetOutputLow(l->rs_port, l->rs_pin);

  // If there is a RW pin indicated, set it low to write
  if (l->rw_pin != 255) {
    XMC_GPIO_SetOutputLow(l->rw_port, l->rw_pin);
  }
  
  if (l->display_function & LCD_8BITMODE) {
    lcd_write8bits(l, value);
  } else {
    lcd_write4bits(l, value >> 4);
    lcd_write4bits(l, value);
  }
}

void lcd_pulse_enable (lcd_t *l) {
  XMC_GPIO_SetOutputLow(l->enable_port, l->enable_pin);
  dwt_delay(1);
  XMC_GPIO_SetOutputHigh(l->enable_port, l->enable_pin);
  dwt_delay(1);    // enable pulse must be >450ns
  XMC_GPIO_SetOutputLow(l->enable_port, l->enable_pin);
  dwt_delay(100);   // commands need > 37us to settle
}

void lcd_write4bits (lcd_t *l, uint8_t value) {
  uint32_t i;

  for (i = 0; i < 4; i++) {
    if ((value >> i) & 0x01)
      XMC_GPIO_SetOutputHigh(l->data_ports[i], l->data_pins[i]);
    else
      XMC_GPIO_SetOutputLow(l->data_ports[i], l->data_pins[i]);
  }

  lcd_pulse_enable(l);
}

void lcd_write8bits (lcd_t *l, uint8_t value) {
  uint32_t i;

  for (i = 0; i < 8; i++) {
    if ((value >> i) & 0x01)
      XMC_GPIO_SetOutputHigh(l->data_ports[i], l->data_pins[i]);
    else
      XMC_GPIO_SetOutputLow(l->data_ports[i], l->data_pins[i]);
  }
  
  lcd_pulse_enable(l);
}

/********************************************************************************
   Lua module functions begin...
 *******************************************************************************/

// Lua: xmc4000.lcd.setup( right-to-left )
// Set right-to-left mode,
static int xmc_lcd_setup( lua_State *L )
{
  // lua_toboolean returns 0 or 1, and returns 0 if the parameter is absent
  unsigned right_to_left = lua_toboolean( L, 1 );  // Default: print left-to-right

  lcd_init_4bits_mode( &xmc_lcd );
  if ( right_to_left )
    lcd_right_to_left( &xmc_lcd );

  return 0;
}

// Lua: xmc4000.lcd.clear()
// Clear the display, reset its shiftedness and put the cursor at 1,1
static int xmc_lcd_clear( lua_State *L )
{
  lcd_clear( &xmc_lcd );
  return 0;
}

// Lua: xmc4000.lcd.home()
// Reset the display's shiftedness and put the cursor at 1,1
static int xmc_lcd_home( lua_State *L )
{
  lcd_home( &xmc_lcd );
  return 0;
}

// Lua: xmc4000.lcd.goto( row, col )
// Move the cursor to the specified row (1 or 2) and column (1-16)
// in the character memory.
static int xmc_lcd_goto( lua_State *L )
{
  int row = luaL_checkinteger( L, 1 );
  int col = luaL_checkinteger( L, 2 );

  if ( row < 1 || row > LCD_ROWS || col < 1 || col > LCD_COLS )
    return luaL_error( L, "row/column must be 1-%d and 1-%d", LCD_ROWS, LCD_COLS );

  lcd_set_cursor( &xmc_lcd, col - 1, row );
  return 0;
}

// Lua: xmc4000.lcd.print( string )
// Send data bytes to the LCD module.
// Usually this will be a string of text or a list of character codes.
// If they pass us integer values <0 or >255, we just use the bottom 8 bits.
static int xmc_lcd_print( lua_State *L )
{
  unsigned argc = lua_gettop( L );  // Number of parameters supplied
  int argn;
  
  for ( argn = 1; argn <= argc; argn ++ )
  {
    switch ( lua_type( L, argn ) )
    {
      case LUA_TNUMBER:
      {
        u8 byte = luaL_checkint( L, argn );
	lcd_write( &xmc_lcd, byte );
      }
      break;

      case LUA_TSTRING:
      {
        size_t len;  // Number of chars in string
        const char *str = luaL_checklstring( L, argn, &len );
	lcd_echo( &xmc_lcd, (u8 *) str, len );
      }
      break;

      default:
        return luaL_typerror( L, argn, "integer or string" );
    }
  }
  return 0;
}

//
// TODO: For later, when we initialize the ADCs.
//
// Return the current state of the pressed buttons as a string containing
// a selection of the letters S, L, R, U, D or an empty string if none are
// currently held down.
/* static int lcd_buttons( lua_State *L ) */
/* { */
/*    u8 code;           // bit code for buttons held */
/*    char string[6];         // Up to 5 buttons and a \0 */
/*    char *stringp = string; // Where to write the next character; */

/*    code = recv_buttons(); */
/*    if( code & LCD_BUTTON_SELECT ) *stringp++ = 'S'; */
/*    if( code & LCD_BUTTON_LEFT   ) *stringp++ = 'L'; */
/*    if( code & LCD_BUTTON_RIGHT  ) *stringp++ = 'R'; */
/*    if( code & LCD_BUTTON_UP     ) *stringp++ = 'U'; */
/*    if( code & LCD_BUTTON_DOWN   ) *stringp++ = 'D'; */
/*    *stringp = '\0'; */

/*    lua_pushstring( L, string ); */

/*    return 1; */
/* } */

// Perform cursor operations, selected by a string parameter,
// as recommended in the Lua Reference Manual, p.58: "luaL_checkoption()"
static int xmc_lcd_cursor( lua_State *L )
{
  static const char const *args[] =
    { "off", "on", "noblink", "blink", NULL };

  switch ( luaL_checkoption( L, 1, NULL, args ) )
  {
  case 0:
    lcd_no_cursor( &xmc_lcd );
    break;
  case 1:
    lcd_cursor( &xmc_lcd );
    break;
  case 2:
    lcd_no_blink( &xmc_lcd );
    break;
  case 3:
    lcd_blink( &xmc_lcd );
    break;
  default:
    return luaL_argerror( L, 1, NULL );
  }
  return 0;
}

// Perform display operations, selected by a string parameter.
// xmc4000.lcd.display( control_string )
static int xmc_lcd_display( lua_State *L )
{
  static const char const *args[] =
    { "off", "on", "left", "right", NULL };

  switch( luaL_checkoption( L, 1, NULL, args ) )
  {
    case 0: lcd_no_display( &xmc_lcd ); break;
    case 1: lcd_display( &xmc_lcd ); break;
    case 2: lcd_scroll_display_left( &xmc_lcd ); break;
    case 3: lcd_scroll_display_right( &xmc_lcd ); break;
    default: return luaL_argerror( L, 1, NULL );
  }
  return 0;
}

// Lua: xmc4000.lcd.definechar( code, glyph )
// code: 0-7
// glyph: a table of up to 8 numbers with values 0-31.
//        If less than 8 are supplied, the bottom rows are blanked.
//        If more than 8 are supplied, the extra are ignored.
static int xmc_lcd_definechar( lua_State *L ) {
  int code;        // The character code we are defining, 0-7
  size_t datalen;  // The number of elements in the glyph table
  size_t line;     // Which line of the char are we defining?
  u8 data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int old_address; // The coded value for the current cursor position

  // First parameter: glyph code to define
  code = luaL_checkint( L, 1 );
  if( code < 0 || code > 7 )
    return luaL_error( L, "user-defined characters have codes 0-7" );

  // Second parameter: table of integer values to define the glyph
  luaL_checktype( L, 2, LUA_TTABLE );
  datalen = lua_objlen( L, 2 );
  // Check all parameters before starting the I2C command.
  if( datalen >= 8 ) datalen = 8;            // Ignore extra parameters
  for( line = 0; line < datalen; line ++ )
  {
    int value;
    lua_rawgeti( L, 2, line + 1 );
    value = luaL_checkint( L, -1 );
    lua_pop( L, 1 );
    data[line] = value;
  }

  lcd_create_char( &xmc_lcd, code, data );

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// mizar32.disp.*() module function map
const LUA_REG_TYPE lcd_map[] =
{
  { LSTRKEY( "setup" ),      LFUNCVAL( xmc_lcd_setup ) },
  { LSTRKEY( "clear" ),      LFUNCVAL( xmc_lcd_clear ) },
  { LSTRKEY( "home" ),       LFUNCVAL( xmc_lcd_home ) },
  { LSTRKEY( "goto" ),       LFUNCVAL( xmc_lcd_goto ) },
  { LSTRKEY( "print" ),      LFUNCVAL( xmc_lcd_print ) },
  { LSTRKEY( "cursor" ),     LFUNCVAL( xmc_lcd_cursor ) },
  { LSTRKEY( "definechar" ), LFUNCVAL( xmc_lcd_definechar ) },
  { LSTRKEY( "display" ),    LFUNCVAL( xmc_lcd_display ) },
  { LNILKEY, LNILVAL }
};
