// Terminal functions

#ifndef __TERM_H__
#define __TERM_H__

#include "type.h"

// ****************************************************************************
// Data types

// Terminal output function
typedef void ( *p_term_out )( u8 );
// Terminal input function
typedef int ( *p_term_in )( int );
// Terminal translate input function
typedef int ( *p_term_translate )( int );

// Terminal input mode (parameter of p_term_in and term_getch())
#define TERM_INPUT_DONT_WAIT      0
#define TERM_INPUT_WAIT           1

// Maximum size on an ANSI sequence
#define TERM_MAX_ANSI_SIZE        14

// ****************************************************************************
// Exported functions

// Terminal initialization
void term_init( unsigned lines, unsigned cols, p_term_out term_out_func, 
                p_term_in term_in_func, p_term_translate term_translate_func );

// Terminal output functions
void term_clrscr(void);
void term_clreol(void);
void term_gotoxy( unsigned x, unsigned y );
void term_up( unsigned delta );
void term_down( unsigned delta );
void term_left( unsigned delta );
void term_right( unsigned delta );
unsigned term_get_lines(void);
unsigned term_get_cols(void);
void term_putch( u8 ch );
void term_putstr( const char* str, unsigned size );
unsigned term_get_cx(void);
unsigned term_get_cy(void);
void term_savepos();
void term_restorepos();
void term_setscroll( unsigned start, unsigned end );

#define TERM_KEYCODES\
  _D( KC_UP ),\
  _D( KC_DOWN ),\
  _D( KC_LEFT ),\
  _D( KC_RIGHT ),\
  _D( KC_HOME ),\
  _D( KC_END ),\
  _D( KC_PAGEUP ),\
  _D( KC_PAGEDOWN ),\
  _D( KC_ENTER ),\
  _D( KC_TAB ),\
  _D( KC_BACKSPACE ),\
  _D( KC_ESC ),\
  _D( KC_CTRL_Z ),\
  _D( KC_CTRL_A ),\
  _D( KC_CTRL_E ),\
  _D( KC_CTRL_C ),\
  _D( KC_CTRL_T ),\
  _D( KC_CTRL_U ),\
  _D( KC_CTRL_K ),\
  _D( KC_DEL ),\
  _D( KC_UNKNOWN )
  
// Terminal input functions
// Keyboard codes
#define _D( x ) x

enum
{
  term_dummy = 255,
  TERM_KEYCODES,
  TERM_FIRST_KEY = KC_UP,
  TERM_LAST_KEY = KC_UNKNOWN
};

int term_getch( int mode );

#endif // #ifndef __TERM_H__
