// Terminal function

#include "term.h"
#include "platform.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "platform_conf.h"
#ifdef BUILD_TERM

// Local variables
static p_term_out term_out;
static p_term_in term_in;
static p_term_translate term_translate;
static unsigned term_num_lines, term_num_cols;
static unsigned term_cx, term_cy;

// *****************************************************************************
// Terminal functions


// Helper function: send the requested string to the terminal

#ifdef __GNUC__
static void term_ansi( const char* fmt, ... )  __attribute__ ((format (printf, 1, 2)));
#endif

static void term_ansi( const char* fmt, ... )
{
  char seq[ TERM_MAX_ANSI_SIZE + 1 ];
  va_list ap;
    
  seq[ TERM_MAX_ANSI_SIZE ] = '\0';
  seq[ 0 ] = '\x1B';
  va_start( ap, fmt );
  vsnprintf( seq + 1, TERM_MAX_ANSI_SIZE - 1, fmt, ap );
  va_end( ap );
  term_putstr( seq, strlen( seq ) );
}

// Clear the screen
void term_clrscr()
{
  term_ansi( "[2J" );
  term_cx = term_cy = 0;
}

void term_savepos()
{
  term_ansi( "7" );
}

void term_restorepos()
{
  term_ansi( "8" );
}

void term_setscroll( unsigned start, unsigned end )
{
  if( ( start > 0 ) && ( end > 0 ) )
    term_ansi( "[%u;%ur", start, end );
  else
    term_ansi( "[r" );
}

// Clear to end of line
void term_clreol()
{
  term_ansi( "[K" );
}

// Move cursor to (x, y)
void term_gotoxy( unsigned x, unsigned y )
{
  term_ansi( "[%u;%uH", y, x );
  term_cx = x;
  term_cy = y;
}

// Move cursor up "delta" lines
void term_up( unsigned delta )
{
  term_ansi( "[%uA", delta );
  term_cy -= delta;
}

// Move cursor down "delta" lines
void term_down( unsigned delta )
{
  term_ansi( "[%uB", delta );
  term_cy += delta;
}

// Move cursor right "delta" chars
void term_right( unsigned delta )
{
  term_ansi( "[%uC", delta );
  term_cx -= delta;
}

// Move cursor left "delta" chars
void term_left( unsigned delta )
{
  term_ansi( "[%uD", delta );
  term_cx += delta;
}

// Return the number of terminal lines
unsigned term_get_lines()
{
  return term_num_lines;
}

// Return the number of terminal columns
unsigned term_get_cols()
{
  return term_num_cols;
}

// Write a character to the terminal
void term_putch( u8 ch )
{
  if( ch == '\n' )
  {
    if( term_cy < term_num_lines )
      term_cy ++;
    term_cx = 0;
  }
  term_out( ch );
}

// Write a string to the terminal
void term_putstr( const char* str, unsigned size )
{
  while( size )
  {
    term_out( *str ++ );
    size --;
  }
}

// Write a string of 
 
// Return the cursor "x" position
unsigned term_get_cx()
{
  return term_cx;
}

// Return the cursor "y" position
unsigned term_get_cy()
{
  return term_cy;
}

// Return a char read from the terminal
// If "mode" is TERM_INPUT_DONT_WAIT, return the char only if it is available,
// otherwise return -1
// Calls the translate function to translate the terminal's physical key codes
// to logical key codes (defined in the term.h header)
int term_getch( int mode )
{
  int ch;
  
  if( ( ch = term_in( mode ) ) == -1 )
    return -1;
  else
    return term_translate( ch );
}

void term_init( unsigned lines, unsigned cols, p_term_out term_out_func, 
                p_term_in term_in_func, p_term_translate term_translate_func )
{
  term_num_lines = lines;
  term_num_cols = cols;
  term_out = term_out_func;
  term_in = term_in_func;
  term_translate = term_translate_func;
  term_cx = term_cy = 0;
}                

#else // #ifdef BUILD_TERM

void term_init( unsigned lines, unsigned cols, p_term_out term_out_func, 
                p_term_in term_in_func, p_term_translate term_translate_func )
{
}

#endif // #ifdef BUILD_TERM
