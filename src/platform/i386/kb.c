/* bkerndev - Bran's Kernel Development Tutorial
*  By:   Brandon F. (friesenb@gmail.com)
*  Desc: Keyboard driver
*
*  Notes: No warranty expressed or implied. Use at own risk. */
#include "common.h"
#include "monitor.h"
#include "isr.h"
#include "type.h"
#include "utils.h"
#include "term.h"

#define RSHIFT 0x36
#define LSHIFT 0x2A
#define CTRL   0x1D
static int shift_pressed, ctrl_pressed;

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
const unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'q', 'w', 'e', 'r', /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   /* Enter key */
    0,      /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,    /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',      /* 49 */
  'm', ',', '.', '/',   0,          /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

const unsigned char shift_kbdus[128] =
{
  0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b', /* Backspace */
  '\t',     /* Tab */
  'Q', 'W', 'E', 'R', /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   /* Enter key */
  0,      /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
  '"', '~',   0,    /* Left shift */
  '|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
  'M', '<', '>', '?',   0,          /* Right shift */
  '*',
  0,  /* Alt */
  ' ',  /* Space bar */
  0,  /* Caps lock */
  0,  /* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,  /* < ... F10 */
  0,  /* 69 - Num lock*/
  0,  /* Scroll Lock */
  0,  /* Home key */
  0,  /* Up Arrow */
  0,  /* Page Up */
  '-',
  0,  /* Left Arrow */
  0,
  0,  /* Right Arrow */
  '+',
  0,  /* 79 - End key*/
  0,  /* Down Arrow */
  0,  /* Page Down */
  0,  /* Insert Key */
  0,  /* Delete Key */
  0,   0,   0,
  0,  /* F11 Key */
  0,  /* F12 Key */
  0,  /* All other keys are undefined */
};

// Keyboard buffer
#define KBUF_SIZE     64
static int kb_buffer[ KBUF_SIZE ];
static u8 r_ptr;
static volatile u8 w_ptr;

// Special keys lookup
static const unsigned char skeys[] = { 0x48, 0x50, 0x4b, 0x4d, 0x47, 0x4f, 0x49, 0x51, 0, 0, 0, 1 };

/* Handles the keyboard interrupt */
void keyboard_handler(registers_t regs)
{
    unsigned char scancode;
    int i;

    /* Read from the keyboard's data buffer */
    /* Ignore 0xE0 (extended indication) */
    if( ( scancode = inb( 0x60 ) ) == 0xE0 )
     return;

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
      scancode &= 0x7F;
      if( ( scancode == RSHIFT ) || ( scancode == LSHIFT ) )
        shift_pressed = 0;
      else if ( scancode == CTRL )
        ctrl_pressed = 0;
    }
    else
    {

      /* Check if this is a special key (will be used by the term module) */    
      for( i = 0; i < sizeof( skeys ) / sizeof( unsigned char ); i ++ )
        if( skeys[ i ] == scancode )
        {
            kb_buffer[ w_ptr ] = TERM_FIRST_KEY + i;
            w_ptr = ( w_ptr + 1 ) % KBUF_SIZE;
            return;
        }

      /* Here, a key was just pressed. Please note that if you
      *  hold a key down, you will get repeated key press
      *  interrupts. */

      /* Just to show you how this works, we simply translate
      *  the keyboard scancode into an ASCII value, and then
      *  display it to the screen. You can get creative and
      *  use some flags to see if a shift is pressed and use a
      *  different layout, or you can add another 128 entries
      *  to the above layout to correspond to 'shift' being
      *  held. If shift is held using the larger lookup table,
      *  you would add 128 to the scancode when you look for it */
      if( ( scancode == RSHIFT ) || ( scancode == LSHIFT ) )
        shift_pressed = 1;
      else if( scancode == CTRL )
        ctrl_pressed = 1;
      else      
      {
        if( ( ( w_ptr + 1 ) % KBUF_SIZE ) != r_ptr )
        {
          unsigned char thechar = shift_pressed ? shift_kbdus[scancode] : kbdus[scancode];
          if( ctrl_pressed ) // Look for CTRL+Z (EOF)
            thechar = ( thechar == 'z' || thechar == 'Z' ) ? STD_CTRLZ_CODE : 0;
          if( thechar != 0 )
          {
            kb_buffer[ w_ptr ] = thechar;
            w_ptr = ( w_ptr + 1 ) % KBUF_SIZE;
          }
        }
      }
    }
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
  register_interrupt_handler(IRQ1, keyboard_handler);
}

// Return a key from the keyboard (blocking!)
int keyboard_getch()
{
  int c;
  
  while( r_ptr == w_ptr );
  c = kb_buffer[ r_ptr ];
  r_ptr = ( r_ptr + 1 ) % KBUF_SIZE;
  return c; 
}

