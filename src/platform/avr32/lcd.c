// eLua module for Mizar32 LCD character display

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"

#include "lcd.h"       
#include "i2c.h"


// Since the LCD firmware currently only runs at up to 20kHz on the I2C bus,
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
static int send_generic(char address, const char *data, int len)
{
  while (len > 0) {
    int nbytes;    // number of bytes sent in this I2C packet

    lcd_start();
    i2c_start_cond();
    i2c_write_byte( address );
    // Mizar32 LCD module has a maximum of 31 bytes per data packet
    nbytes = 0;
    while ( len > 0 && nbytes < 31 ) {
      i2c_write_byte( *data++ );
      nbytes++; len--;
    }
    i2c_stop_cond();
    lcd_stop();
  }
  return 0;
}

// Send a single command byte
static int send_command(const char command)
{
  return send_generic(LCD_CMD, &command, 1);
}

// Send multiple command bytes as one message
static int send_commands(const char *commands, int len)
{
  return send_generic(LCD_CMD, commands, len);
}

// Send data bytes
// This is used for printing data and for programming the user-defining chars
static int send_data(const char *data, int len)
{
  return send_generic(LCD_DATA, data, len);
}


// *** Lua module functions begin... ***


// Turning the display on can only be achieved by simultaneously specifying the
// cursor type, so we have to remember what type of cursor they last set.
// Similarly, if they have turned the display off then set the cursor, this
// shouldn-t turn the display on.

// Power-on setting is no cursor
#define DEFAULT_CURSOR_TYPE   LCD_CMD_CURSOR_NONE

static char cursor_type = DEFAULT_CURSOR_TYPE;
static char display_is_off = 0;     // Have they called display("off")?


// Should we try to maintain the current cursor position across a definechar()?
// Unfortunately we can't read the current cursor position, and definechar()
// destroys it. The LCD controller does have a read-cursor-position primitive
// but the current PIC firmware doesn't pass this on as an I2C read.2
// So we have to track the cursor position. Yuk.
// The only relief is that we don't have to track the display scrolling.
// Adds 284 bytes of code to the executable.
//
// If, one day, we can read the LCD cursor position through the PIC firmware
// we can remove all this stuff.
#define KEEP_CURSOR_POSITION 1

#ifdef KEEP_CURSOR_POSITION
// Where is the cursor in the character memory?  Required ONLY to be able to
// restore the cursor position when they define a character :-/
static int current_row = 0;        // 0 or 1
static int current_column = 0;     // 0-39 (though it over- and underflows)
static int current_direction = 1;  // left-to-right. -1 is right-to-left
#endif


// Lua: mizar32.disp.reset()
// Ensure the display is in a known initial state
static int lcd_reset( lua_State *L )
{
  // Initialise the display to a known state
  static const char reset[] = {
    0	/* reset */
  };

  // Set the static variables
  cursor_type = DEFAULT_CURSOR_TYPE;
  display_is_off = 0;
#ifdef KEEP_CURSOR_POSITION
  current_row = current_column = 0;
  current_direction = 1;
#endif

  return send_commands( reset, sizeof( reset ) );
}

// "Entry mode" function.

// Lua: mizar32.disp.setup( shift_display, right-to-left )
// Set right-to-left mode,
static int lcd_setup( lua_State *L )
{
  // lua_toboolean returns 0 or 1, and returns 0 if the parameter is absent
  unsigned shift_display = lua_toboolean( L, 1 );  // Default: move cursor
  unsigned right_to_left = lua_toboolean( L, 2 );  // Default: print left-to-right

#ifdef KEEP_CURSOR_POSITION
  current_direction = right_to_left ? -1 : 1;
#endif

  return send_command( LCD_CMD_ENTRYMODE + shift_display +
                       (!right_to_left) * 2 );
}

// Lua: mizar32.disp.clear()
// Clear the display, reset its shiftedness and put the cursor at 1,1
static int lcd_clear( lua_State *L )
{
#ifdef KEEP_CURSOR_POSITION
  current_row = current_column = 0;
#endif
  return send_command( LCD_CMD_CLEAR );
}

// Lua: mizar32.disp.home()
// Reset the display's shiftedness and put the cursor at 1,1
static int lcd_home(lua_State *L)
{
#ifdef KEEP_CURSOR_POSITION
  current_row = current_column = 0;
#endif
  return send_command( LCD_CMD_HOME );
}

// Lua: mizar32.disp.goto( row, col )
// Move the cursor to the specified row (1 or 2) and column (1-40)
// in the character memory.
static int lcd_goto(lua_State *L)
{
  unsigned row = luaL_checkinteger( L, 1 );
  unsigned col = luaL_checkinteger( L, 2 );
  unsigned address;

  if ( row < 1 || row > 2 || col < 1 || col > 40 )
    return luaL_error( L, "row/column must be 1-2 and 1-40" );

#ifdef KEEP_CURSOR_POSITION
  current_row = row - 1;
  current_column = col - 1;
#endif
  address = ( row - 1 ) * 0x40 + ( col - 1 ) ;
  return send_command( LCD_CMD_DDADDR + address );
}

// Lua: mizar32.disp.print( string )
// Send data bytes to the LCD module.
// Usually this will be a string of text or a list of character codes.
// If they pass us integer values <0 or >255, we just use the bottom 8 bits.

#ifdef KEEP_CURSOR_POSITION
// Adjust current cursor position by N printed characters.
// Written for shortest code.
static void current_print(int n)
{
  current_column += current_direction * n;
  if (current_column < 0 || current_column >= 40) {
    current_row = ! current_row;
    current_column -= 40 * current_direction;
  }
}
#endif

static int lcd_print(lua_State *L)
{
  unsigned argc = lua_gettop( L );  // Number of parameters supplied
  int argn;
  
  for ( argn = 1; argn <= argc; argn ++ )
  {
    switch (lua_type( L, argn ) )
    {
      case LUA_TNUMBER:
      {
        char byte = luaL_checkint( L, argn );

#ifdef KEEP_CURSOR_POSITION
        current_print(1);
#endif
        send_data(&byte, (size_t) 1);
      }
      break;

      case LUA_TSTRING:
      {
        size_t len;  // Number of chars in string
        const char *str = luaL_checklstring( L, argn, &len );

#ifdef KEEP_CURSOR_POSITION
        current_print(len);
#endif
        send_data(str, len);
      }
      break;

      default:
        return luaL_typerror( L, argn, "integer or string" );
    }
  }
  return 0;
}


// "Display on/off control" functions

// Helper function to set a cursor type if the display is on,
// or to remember which cursor they asked for, to be able to set it
// when they turn the display on.
static int set_cursor( char command_byte )
{
  cursor_type = command_byte;

  // Setting cursor type always turns the display on
  if (display_is_off)
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
    return set_cursor( LCD_CMD_CURSOR_NONE );
  case 1:
    return set_cursor( LCD_CMD_CURSOR_BLOCK );
  case 2:
    return set_cursor( LCD_CMD_CURSOR_LINE );

  case 3: 
#ifdef KEEP_CURSOR_POSITION
    if (--current_column < 0) {
      current_row = !current_row;
      current_column = 39;
    }
#endif
    return send_command( LCD_CMD_SHIFT_CURSOR_LEFT );

  case 4:
#ifdef KEEP_CURSOR_POSITION
    if (++current_column >= 40) {
      current_row = !current_row;
      current_column = 0;
    }
#endif
    return send_command( LCD_CMD_SHIFT_CURSOR_RIGHT );

  default: return luaL_argerror( L, 1, NULL );
  }
}

// Perform display operations, selected by a string parameter.
static int lcd_display( lua_State *L )
{
  static const char const *args[] =
    { "off", "on", "left", "right", NULL };

  switch ( luaL_checkoption( L, 1, NULL, args ) )
  {
  case 0: display_is_off = 1;
	  return send_command( LCD_CMD_DISPLAY_OFF );
  case 1: display_is_off = 0;
          return send_command( cursor_type );	// Turns display on
  case 2: return send_command( LCD_CMD_SHIFT_DISPLAY_LEFT );
  case 3: return send_command( LCD_CMD_SHIFT_DISPLAY_RIGHT );
  default: return luaL_argerror( L, 1, NULL );
  }
}

// Lua: mizar32.disp.definechar( code, glyph )
// code: 0-7
// glyph: a table of up to 8 numbers with values 0-31.
//        If less than 8 are supplied, the bottom rows are blanked.
//        If more than 8 are supplied, the extra are ignored.

static int lcd_definechar( lua_State *L ) {
  int code;        // The character code we are defining, 0-7
  size_t datalen;  // The number of elements in the glyph table
  size_t line;     // Which line of the char are we defining?
  char data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#ifdef KEEP_CURSOR_POSITION
  int old_column = current_column, old_row = current_row;
#endif

  // First parameter: glyph code to define
  code = luaL_checkint( L, 1 );
  if( code < 0 || code > 7 )
    return luaL_error( L, "user-defined characters have codes 0-7");

  // Second parameter: table of integer values to define the glyph
  luaL_checktype( L, 2, LUA_TTABLE );
  datalen = lua_objlen( L, 2 );
  // Check all parameters before starting the I2C command.
  if( datalen >= 8) datalen = 8;            // Ignore extra parameters
  for( line = 0; line < datalen; line ++ )
  {
    int value;
    lua_rawgeti( L, 2, line + 1 );
    value = luaL_checkint( L, -1 );
    lua_pop( L, 1 );
    data[line] = value;
  }

  send_command( LCD_CMD_CGADDR + code * 8 );
  send_data( data, sizeof(data) );

#ifdef KEEP_CURSOR_POSITION
  // Move back to where we were
  current_row = old_row; current_column = old_column;
  return send_command( LCD_CMD_DDADDR + current_row * 0x40 + current_column );
#else
  // Sadly, we cannot save and restore the current cursor position
  // so return to the home position.
  return send_command( LCD_CMD_DDADDR );
#endif
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
  { LNILKEY, LNILVAL }
};
