// monitor.c -- Defines functions for writing to the monitor.
//             heavily based on Bran's kernel development tutorials,
//             but rewritten for JamesM's kernel tutorials.

#include "monitor.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "term.h"
#include "utils.h"

// The VGA framebuffer starts at 0xB8000.
u16int *video_memory = (u16int *)0xB8000;
// Stores the cursor position.
u8int cursor_x = 0;
u8int cursor_y = 0;

// Updates the hardware cursor.
static void move_cursor()
{
    cursor_y = UMIN( cursor_y, 24 );
    cursor_x = UMIN( cursor_x, 79 );
    // The screen is 80 characters wide...
    u16int cursorLocation = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void scroll()
{

    // Get a space character with the default colour attributes.
    u8int attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    u16int blank = 0x20 /* space */ | (attributeByte << 8);

    // Row 25 is the end, this means we need to scroll up
    if(cursor_y >= 25)
    {
        // Move the current text chunk that makes up the screen
        // back in the buffer by a line
        int i;
        for (i = 0*80; i < 24*80; i++)
        {
            video_memory[i] = video_memory[i+80];
        }

        // The last line should now be blank. Do this by writing
        // 80 spaces to it.
        for (i = 24*80; i < 25*80; i++)
        {
            video_memory[i] = blank;
        }
        // The cursor should now be on the last line.
        cursor_y = 24;
    }
}

// ANSI 'state machine'
static int monitor_reading_ansi;
static int monitor_ansi_count;
static char monitor_ansi_inbuf[ TERM_MAX_ANSI_SIZE + 1 ];

// ANSI operations and structure data
enum
{
  ANSI_SEQ_CLRSCR,
  ANSI_SEQ_CLREOL,
  ANSI_SEQ_GOTOXY,
  ANSI_SEQ_UP,
  ANSI_SEQ_DOWN,
  ANSI_SEQ_RIGHT,
  ANSI_SEQ_LEFT
};

typedef struct
{
  int op;
  int p1, p2; 
} ansi_op;

// Convert an ASCII escape sequence to an operation we can understand
static int monitor_cvt_escape( const char* inbuf, ansi_op* res )
{
  const char *p = inbuf;
  char last = inbuf[ strlen( inbuf ) - 1 ];

  if( *p++ != '\x1B' )
    return 0;
  if( *p++ != '[' )
    return 0;
  res->op = res->p1 = res->p2 = 0;
  switch( last )
  {
    case 'J': // clrscr
      if( *p != '2' )
        return 0;
      res->op = ANSI_SEQ_CLRSCR;
      break;

    case 'K': // clreol
      res->op = ANSI_SEQ_CLREOL;
      break;

    case 'H': // gotoxy
      res->op = ANSI_SEQ_GOTOXY;
      if( *p != 'H' )
        sscanf( p, "%d;%d", &res->p1, &res->p2 );
      break;

    case 'A': // up
    case 'B': // down
    case 'C': // right
    case 'D': // left
      res->op = last - 'A' + ANSI_SEQ_UP;
      sscanf( p, "%d", &res->p1 );
      break;
  }
  return 1;
}

void monitor_clear();
// Writes a single character out to the screen.
void monitor_put(char c)
{
    // The background colour is black (0), the foreground is white (15).
    u8int backColour = 0;
    u8int foreColour = 15;

    // Take care of the ANSI state machine
    if( c == '\x1B' )
    {
      monitor_reading_ansi = 1;
      monitor_ansi_count = 0;
      monitor_ansi_inbuf[ monitor_ansi_count ++ ] = c; 
      return;
    }

    // The attribute byte is made up of two nibbles - the lower being the 
    // foreground colour, and the upper the background colour.
    u8int  attributeByte = (backColour << 4) | (foreColour & 0x0F);
    // The attribute byte is the top 8 bits of the word we have to send to the
    // VGA board.
    u16int attribute = attributeByte << 8;
    u16int *location;
    u8int prev;

    if( monitor_reading_ansi )
    {
      monitor_ansi_inbuf[ monitor_ansi_count ++ ] = c;
      if( isalpha( c ) )
      {
        monitor_ansi_inbuf[ monitor_ansi_count ] = '\0';
        ansi_op op;
        if( monitor_cvt_escape( monitor_ansi_inbuf, &op ) )
        {
          // Interpret out sequence
          switch( op.op )
          {
            case ANSI_SEQ_CLRSCR:
              monitor_clear();
              break;

            case ANSI_SEQ_CLREOL:
              prev = cursor_x;
              while( cursor_x++ < 80 )
              {
                location = video_memory + (cursor_y*80 + cursor_x);      
                *location = ' ' | attribute;
              }
              cursor_x = prev;
              break;

            case ANSI_SEQ_GOTOXY:
              cursor_y = ( u8int )op.p1 - 1;
              cursor_x = ( u8int )op.p2 - 1;
              move_cursor();
              break;

            case ANSI_SEQ_UP:
            case ANSI_SEQ_LEFT:
            case ANSI_SEQ_RIGHT:
            case ANSI_SEQ_DOWN:
              {
                int xm = op.op == ANSI_SEQ_LEFT ? -1 : op.op == ANSI_SEQ_RIGHT ? 1 : 0;
                int ym = op.op == ANSI_SEQ_UP ? -1 : op.op == ANSI_SEQ_DOWN ? 1 : 0;
                cursor_x += xm * op.p1;
                cursor_y += ym * op.p1;
                move_cursor();
                break;
              }
           }
        }
        monitor_reading_ansi = 0;
      }
      return;
    }

    // Handle a backspace, by moving the cursor back one space
    if (c == 0x08 && cursor_x)
    {
        cursor_x--;
    }

    // Handle a tab by increasing the cursor's X, but only to a point
    // where it is divisible by 2.
    else if (c == 0x09)
    {
        cursor_x = (cursor_x+2) & ~(2-1);
    }

    // Handle carriage return
    else if (c == '\r')
    {
        cursor_x = 0;
    }

    // Handle newline by moving cursor back to left and increasing the row
    else if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    // Handle any other printable character.
    else if(c >= ' ')
    {
        location = video_memory + (cursor_y*80 + cursor_x);
        *location = c | attribute;
        cursor_x++;
    }

    // Check if we need to insert a new line because we have reached the end
    // of the screen.
    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y ++;
    }

    // Scroll the screen if needed.
    scroll();
    // Move the hardware cursor.
    move_cursor();

}

// Clears the screen, by copying lots of spaces to the framebuffer.
void monitor_clear()
{
    // Make an attribute byte for the default colours
    u8int attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    u16int blank = 0x20 /* space */ | (attributeByte << 8);

    int i;
    for (i = 0; i < 80*25; i++)
    {
        video_memory[i] = blank;
    }

    // Move the hardware cursor back to the start.
    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

// Outputs a null-terminated ASCII string to the monitor.
void monitor_write(char *c)
{
    int i = 0;
    while (c[i])
    {
        monitor_put(c[i++]);
    }
}

void monitor_write_hex(u32int n)
{
    s32int tmp;

    char noZeroes = 1;

    int i;
    for (i = 28; i > 0; i -= 4)
    {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0)
        {
            continue;
        }
    
        if (tmp >= 0xA)
        {
            noZeroes = 0;
            monitor_put (tmp-0xA+'a' );
        }
        else
        {
            noZeroes = 0;
            monitor_put( tmp+'0' );
        }
    }
  
    tmp = n & 0xF;
    if (tmp >= 0xA)
    {
        monitor_put (tmp-0xA+'a');
    }
    else
    {
        monitor_put (tmp+'0');
    }

}

void monitor_write_dec(u32int n)
{

    if (n == 0)
    {
        monitor_put('0');
        return;
    }

    s32int acc = n;
    char c[32];
    int i = 0;
    while (acc > 0)
    {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }
    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0)
    {
        c2[i--] = c[j++];
    }
    monitor_write(c2);

}
