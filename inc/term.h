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
typedef int ( *p_term_translate )( u8 );

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
void term_clrscr();
void term_clreol();
void term_gotoxy( unsigned x, unsigned y );
void term_up( unsigned delta );
void term_down( unsigned delta );
void term_left( unsigned delta );
void term_right( unsigned delta );
unsigned term_get_lines();
unsigned term_get_cols();
void term_putch( u8 ch );
void term_putstr( const char* str );
unsigned term_get_cx();
unsigned term_get_cy();

// Terminal input functions
// Keyboard codes
enum
{
  KC_UP = 256,
  KC_DOWN,
  KC_LEFT,
  KC_RIGHT,
  KC_HOME,
  KC_END,
  KC_PAGEUP,
  KC_PAGEDOWN,
  KC_ENTER,
  KC_TAB,
  KC_BACKSPACE,
  KC_UNKNOWN
};

int term_getch( int mode );

#endif // #ifndef __TERM_H__
