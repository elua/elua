// eLua module for Mizar32 LCD character display

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"

#include "lcd.h"       
#include "i2c.h"


// The LCD firmware only runs at up to 50kHz on the I2C bus, so
// we bracket all I2C packets to the LCD module with two functions
// to be able to save, change and restore the I2C clock rate to what it was
// before.

// Declarations to save/restore the I2C clock rate
extern u32 i2c_delay;
static u32 old_i2c_delay;

static void lcd_start()
{
  old_i2c_delay = i2c_delay;
  i2c_delay = REQ_CPU_FREQ / LCD_BUS_FREQ / 2;
}

static void lcd_stop()
{
  i2c_delay = old_i2c_delay;
}


// Low-level functions to send LCD commands or data.
// The command and data packets differ only in the slave address used,
// so we coalesce them into a single function, generating smaller code.
//
// All three are designed to be used in a tail call:
//   return send_generic( data, len );

// Send a command or data packet.
// "address" is LCD_CMD for LCD commands, LCD_DATA for LCD data.
static int send_generic( u8 address, const u8 *data, int len )
{
  lcd_start();
  i2c_send( address, data, len, true );
  lcd_stop();
  return 0;
}

// Send an I2C read-data command and return the answer.
// "address" is LCD_GETPOS to read the cursor position,
//              LCD_BUTTONS for to read the buttons.
// The answer is always a single byte.
// Returns the number of bytes read (== 1) or 0 f the slave did not
// acknowledge its address.
static int recv_generic( u8 address )
{
  u8 retval;

  lcd_start();
  if( i2c_recv( address, &retval, 1, true ) < 0 ) {
    // The address was not acknowledged, so no slave is present.
    // There is no way to signal this to the Lua layer, so return a
    // harmless value (meaning no buttons pressed or cursor at (1,1)).
    retval = 0;
  }
  lcd_stop();

  return retval;
}

// Send a command byte
static int send_command( const u8 command )
{
  return send_generic( LCD_CMD, &command, 1 );
}

// Send data bytes
// This is used for printing data and for programming the user-defining chars
static int send_data( const u8 *data, int len )
{
  return send_generic( LCD_DATA, data, len );
}

// Return the current value of the address counter.
static u8 recv_address_counter()
{
  return recv_generic( LCD_GETPOS );
}

// Return the current state of the buttons, a bit mask in the bottom 5 bits
// of a byte.
static u8 recv_buttons()
{
  return recv_generic( LCD_BUTTONS );
}

// Turning the display on can only be achieved by simultaneously specifying the
// cursor type, so we have to remember what type of cursor they last set.
// Similarly, if they have turned the display off then set the cursor, this
// shouldn't turn the display on.

// Power-on setting is no cursor
#define DEFAULT_CURSOR_TYPE   LCD_CMD_CURSOR_NONE

static u8 cursor_type = DEFAULT_CURSOR_TYPE;
static u8 display_is_off = 0;     // Have they called display("off")?


// *** Lua module functions begin... ***


// Lua: mizar32.disp.reset()
// Ensure the display is in a known initial state
static int lcd_reset( lua_State *L )
{
  // Set the static variables
  cursor_type = DEFAULT_CURSOR_TYPE;
  display_is_off = 0;

  send_command( LCD_CMD_RESET );
  return 0;
}

// "Entry mode" function.

// Lua: mizar32.disp.setup( shift_display, right-to-left )
// Set right-to-left mode,
static int lcd_setup( lua_State *L )
{
  // lua_toboolean returns 0 or 1, and returns 0 if the parameter is absent
  unsigned shift_display = lua_toboolean( L, 1 );  // Default: move cursor
  unsigned right_to_left = lua_toboolean( L, 2 );  // Default: print left-to-right

  send_command( LCD_CMD_ENTRYMODE + shift_display +
                ( ! right_to_left ) * 2 );
  return 0;
}

// Lua: mizar32.disp.clear()
// Clear the display, reset its shiftedness and put the cursor at 1,1
static int lcd_clear( lua_State *L )
{
  send_command( LCD_CMD_CLEAR );
  return 0;
}

// Lua: mizar32.disp.home()
// Reset the display's shiftedness and put the cursor at 1,1
static int lcd_home( lua_State *L )
{
  send_command( LCD_CMD_HOME );
  return 0;
}

// Lua: mizar32.disp.goto( row, col )
// Move the cursor to the specified row (1 or 2) and column (1-40)
// in the character memory.
static int lcd_goto( lua_State *L )
{
  int row = luaL_checkinteger( L, 1 );
  int col = luaL_checkinteger( L, 2 );
  unsigned address;

  if ( row < 1 || row > 2 || col < 1 || col > 40 )
    return luaL_error( L, "row/column must be 1-2 and 1-40" );

  address = ( row - 1 ) * 0x40 + ( col - 1 ) ;
  send_command( (u8) (LCD_CMD_DDADDR + address) );
  return 0;
}

// Lua: mizar32.disp.print( string )
// Send data bytes to the LCD module.
// Usually this will be a string of text or a list of character codes.
// If they pass us integer values <0 or >255, we just use the bottom 8 bits.

static int lcd_print( lua_State *L )
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
        send_data( &byte, 1 );
      }
      break;

      case LUA_TSTRING:
      {
        size_t len;  // Number of chars in string
        const char *str = luaL_checklstring( L, argn, &len );
        send_data( (u8 *) str, len );
      }
      break;

      default:
        return luaL_typerror( L, argn, "integer or string" );
    }
  }
  return 0;
}

// Return the cursor position as row and column in the ranges 1-2 and 1-40
// The bottom 7-bits of addr are the contents of the address counter.
// The Ampire datasheet says:
//   0x00-0x0F for the first line of DDRAM (presumably 0..39 really),
//   0x40-0x4F for the second line of DDRAM (presumably 64..(64+39) really)
// The top bit (128) is the "Busy Flag", which should always be 0.

static int lcd_getpos( lua_State *L )
{
   u8 addr = recv_address_counter();
   lua_pushinteger( L, (lua_Integer) ( (addr & 0x40) ? 2 : 1 ) );  // row
   lua_pushinteger( L, (lua_Integer) ( (addr & 0x3F) + 1 ) );      // column
   return 2;
}

// Return the current state of the pressed buttons as a string containing
// a selection of the letters S, L, R, U, D or an empty string if none are
// currently held down.

static int lcd_buttons( lua_State *L )
{
   u8 code;           // bit code for buttons held
   char string[6];         // Up to 5 buttons and a \0
   char *stringp = string; // Where to write the next character;

   code = recv_buttons();
   if( code & LCD_BUTTON_SELECT ) *stringp++ = 'S';
   if( code & LCD_BUTTON_LEFT   ) *stringp++ = 'L';
   if( code & LCD_BUTTON_RIGHT  ) *stringp++ = 'R';
   if( code & LCD_BUTTON_UP     ) *stringp++ = 'U';
   if( code & LCD_BUTTON_DOWN   ) *stringp++ = 'D';
   *stringp = '\0';

   lua_pushstring( L, string );

   return 1;
}


// "Display on/off control" functions

// Helper function to set a cursor type if the display is on,
// or to remember which cursor they asked for, to be able to set it
// when they turn the display on.
static int set_cursor( u8 command_byte )
{
  cursor_type = command_byte;

  // Setting cursor type always turns the display on
  if ( display_is_off )
    return 0;
  else
    return send_command( cursor_type );
}

// Perform cursor operations, selected by a string parameter,
// as recommended in the Lua Reference Manual, p.58: "luaL_checkoption()"
static int lcd_cursor( lua_State *L )
{
  static const char const *args[] =
    { "none", "block", "line", "left", "right", NULL };

  switch ( luaL_checkoption( L, 1, NULL, args ) )
  {
  case 0:
    set_cursor( LCD_CMD_CURSOR_NONE );
    break;
  case 1:
    set_cursor( LCD_CMD_CURSOR_BLOCK );
    break;
  case 2:
    set_cursor( LCD_CMD_CURSOR_LINE );
    break;

  case 3: 
    send_command( LCD_CMD_SHIFT_CURSOR_LEFT );
    break;
  case 4:
    send_command( LCD_CMD_SHIFT_CURSOR_RIGHT );
    break;

  default:
    return luaL_argerror( L, 1, NULL );
  }
  return 0;
}

// Perform display operations, selected by a string parameter.
static int lcd_display( lua_State *L )
{
  static const char const *args[] =
    { "off", "on", "left", "right", NULL };

  switch ( luaL_checkoption( L, 1, NULL, args ) )
  {
  case 0: display_is_off = 1;
	  send_command( LCD_CMD_DISPLAY_OFF );
          break;
  case 1: display_is_off = 0;
          send_command( cursor_type );	// Turns display on
          break;
  case 2: send_command( LCD_CMD_SHIFT_DISPLAY_LEFT );
          break;
  case 3: send_command( LCD_CMD_SHIFT_DISPLAY_RIGHT );
          break;
  default: return luaL_argerror( L, 1, NULL );
  }
  return 0;
}

// Lua: mizar32.disp.definechar( code, glyph )
// code: 0-7
// glyph: a table of up to 8 numbers with values 0-31.
//        If less than 8 are supplied, the bottom rows are blanked.
//        If more than 8 are supplied, the extra are ignored.
// The current cursor position in the character display RAM is preserved.

static int lcd_definechar( lua_State *L ) {
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

  old_address = recv_address_counter();

  send_command( LCD_CMD_CGADDR + code * 8 );
  send_data( data, sizeof( data ) );

  // Move back to where we were
  send_command( LCD_CMD_DDADDR + old_address );

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// mizar32.disp.*() module function map
const LUA_REG_TYPE lcd_map[] =
{
  { LSTRKEY( "reset" ),      LFUNCVAL( lcd_reset ) },
  { LSTRKEY( "setup" ),      LFUNCVAL( lcd_setup ) },
  { LSTRKEY( "clear" ),      LFUNCVAL( lcd_clear ) },
  { LSTRKEY( "home" ),       LFUNCVAL( lcd_home ) },
  { LSTRKEY( "goto" ),       LFUNCVAL( lcd_goto ) },
  { LSTRKEY( "print" ),      LFUNCVAL( lcd_print ) },
  { LSTRKEY( "definechar" ), LFUNCVAL( lcd_definechar ) },
  { LSTRKEY( "cursor" ),     LFUNCVAL( lcd_cursor ) },
  { LSTRKEY( "display" ),    LFUNCVAL( lcd_display ) },
  { LSTRKEY( "getpos" ),     LFUNCVAL( lcd_getpos ) } ,
  { LSTRKEY( "buttons" ),    LFUNCVAL( lcd_buttons ) } ,
  { LNILKEY, LNILVAL }
};
