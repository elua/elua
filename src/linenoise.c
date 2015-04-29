/* linenoise.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * You can find the latest source code at:
 * 
 *   http://github.com/antirez/linenoise
 *
 * Does a number of crazy assumptions that happen to be true in 99.9999% of
 * the 2010 UNIX computers around.
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Todo list:
 * - Switch to gets() if $TERM is something we can't support.
 * - Filter bogus Ctrl+<char> combinations.
 * - Win32 support
 *
 * Bloat:
 * - Completion?
 * - History search like Ctrl+r in readline?
 *
 * List of escape sequences used by this program, we do everything just
 * with three sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * CHA (Cursor Horizontal Absolute)
 *    Sequence: ESC [ n G
 *    Effect: moves cursor to column n
 *
 * EL (Erase Line)
 *    Sequence: ESC [ n K
 *    Effect: if n is 0 or missing, clear from cursor to end of line
 *    Effect: if n is 1, clear from beginning of line to cursor
 *    Effect: if n is 2, clear entire line
 *
 * CUF (CUrsor Forward)
 *    Sequence: ESC [ n C
 *    Effect: moves cursor forward of n chars
 *
 * [eLua] code adapted to eLua by bogdanm
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "term.h"
#include "linenoise.h"
#include "platform_conf.h"

#ifdef BUILD_LINENOISE

#define LINENOISE_CTRL_C                    ( -2 )
#define LINENOISE_PUSH_EMPTY                1
#define LINENOISE_DONT_PUSH_EMPTY           0

static const int history_max_lengths[ LINENOISE_TOTAL_COMPONENTS ] = { LINENOISE_HISTORY_SIZE_LUA, LINENOISE_HISTORY_SIZE_SHELL };
static int history_lengths[ LINENOISE_TOTAL_COMPONENTS ];
static char **histories[ LINENOISE_TOTAL_COMPONENTS ];

static int linenoise_internal_addhistory( int id, const char *line, int force_empty );

void linenoise_cleanup( int id ) 
{
  int j;
  
#ifdef LINENOISE_AUTOSAVE_FNAME
  if( id == LINENOISE_ID_LUA )
  {
    if( linenoise_savehistory( id, LINENOISE_AUTOSAVE_FNAME ) == 0 )
      printf( "History saved to %s.\n", LINENOISE_AUTOSAVE_FNAME );
    else
      printf( "Unable to save history to %s.\n", LINENOISE_AUTOSAVE_FNAME );
  }    
#endif   
  if( histories[ id ] ) 
  {
    for( j = 0; j < history_lengths[ id ]; j++ )
      free( histories[ id ][ j ] );
    free( histories[ id ] );
    histories[ id ] = NULL;
  }
  history_lengths[ id ] = 0;   
}

#define MAX_SEQ_LEN           16
static void refreshLine(const char *prompt, char *buf, size_t len, size_t pos, size_t cols) {
    char seq[MAX_SEQ_LEN];
    size_t plen = strlen(prompt);
    
    while((plen+pos) >= cols) {
        buf++;
        len--;
        pos--;
    }
    while (plen+len > cols) {
        len--;
    }

    /* Cursor to left edge */
    snprintf(seq,MAX_SEQ_LEN,"\r");
    term_putstr( seq, strlen( seq ) );
    /* Write the prompt and the current buffer content */
    term_putstr( prompt, strlen( prompt ) );
    term_putstr( buf, len );
    /* Erase to right */
    snprintf(seq,MAX_SEQ_LEN,"\x1b[0K");
    term_putstr( seq, strlen( seq ) );
    /* Move cursor to original position. */
    snprintf(seq,MAX_SEQ_LEN,"\r\x1b[%dC", (int)(pos+plen));
    term_putstr( seq, strlen( seq ) );
}

static int linenoisePrompt(int id, char *buf, size_t buflen, const char *prompt) {
    size_t plen = strlen(prompt);
    size_t pos = 0;
    size_t len = 0;
    size_t cols = TERM_COLS;
    int history_index = 0;
    
    buf[0] = '\0';
    buflen--; /* Make sure there is always space for the nulterm */

    /* The latest history entry is always our current buffer, that
     * initially is just an empty string. */
    linenoise_internal_addhistory( id, "", LINENOISE_PUSH_EMPTY );
    
    term_putstr( prompt, plen );
    while(1) {
        int c;

        c = term_getch( TERM_INPUT_WAIT );
        
        switch(c) 
        {
          case KC_ENTER:
          case KC_CTRL_C:
          case KC_CTRL_Z:
            history_lengths[ id ] --;
            free( histories[ id ][history_lengths[ id ]] );
            if( c == KC_CTRL_C )
              return LINENOISE_CTRL_C;
            else if( c == KC_CTRL_Z )
              return -1;
            return len;      
                        
         case KC_BACKSPACE:
            if (pos > 0 && len > 0) 
            {
              memmove(buf+pos-1,buf+pos,len-pos);
              pos--;
              len--;
              buf[len] = '\0';
              refreshLine(prompt,buf,len,pos,cols);
            }
            break;
             
         case KC_CTRL_T:    /* ctrl-t */
            // bogdanm: this seems to be rather useless and also a bit buggy,
            // so it's not enabled
/*            if (pos > 0 && pos < len) {
                int aux = buf[pos-1];
                buf[pos-1] = buf[pos];
                buf[pos] = aux;
                if (pos != len-1) pos++;
                refreshLine(prompt,buf,len,pos,cols);
            }*/
            break;
            
        case KC_LEFT:
            /* left arrow */
            if (pos > 0) 
            {
              pos--;
              refreshLine(prompt,buf,len,pos,cols);
            }
            break;
                
        case KC_RIGHT:
            /* right arrow */
            if (pos != len) 
            {
              pos++;
              refreshLine(prompt,buf,len,pos,cols);
            }
            break;
                
       case KC_UP:
       case KC_DOWN:
            /* up and down arrow: history */
            if (history_lengths[ id ] > 1) 
            {
              /* Update the current history entry before to
               * overwrite it with tne next one. */
              free(histories[ id ][history_lengths[ id ]-1-history_index]);
              histories[ id ][history_lengths[ id ]-1-history_index] = strdup(buf);
              /* Show the new entry */
              history_index += (c == KC_UP) ? 1 : -1;
              if (history_index < 0) 
              {
                history_index = 0;
                break;
              } else if (history_index >= history_lengths[ id ]) 
              {
                history_index = history_lengths[ id ]-1;
                break;
              }
              strncpy(buf,histories[ id ][history_lengths[ id ]-1-history_index],buflen);
              buf[buflen] = '\0';
              len = pos = strlen(buf);
              refreshLine(prompt,buf,len,pos,cols);
            }
            break;
                
        case KC_DEL:           
            /* delete */
            if (len > 0 && pos < len) 
            {
              memmove(buf+pos,buf+pos+1,len-pos-1);
              len--;
              buf[len] = '\0';
              refreshLine(prompt,buf,len,pos,cols);
            }    
            break;
            
        case KC_HOME: /* Ctrl+a, go to the start of the line */
            pos = 0;
            refreshLine(prompt,buf,len,pos,cols);
            break;
            
        case KC_END: /* ctrl+e, go to the end of the line */
            pos = len;
            refreshLine(prompt,buf,len,pos,cols);
            break;
            
        case KC_CTRL_U: /* Ctrl+u, delete the whole line. */
            buf[0] = '\0';
            pos = len = 0;
            refreshLine(prompt,buf,len,pos,cols);
            break;
            
        case KC_CTRL_K: /* Ctrl+k, delete from current to end of line. */
            buf[pos] = '\0';
            len = pos;
            refreshLine(prompt,buf,len,pos,cols);
            break;
                        
        default:
            if( isprint( c ) && len < buflen )
            {
              if(len == pos) 
              {
                buf[pos] = c;
                pos++;
                len++;
                buf[len] = '\0';
                if (plen+len < cols) 
                {
                  /* Avoid a full update of the line in the
                   * trivial case. */
                  term_putch( c );
                } 
                else 
                {
                  refreshLine(prompt,buf,len,pos,cols);
                }
              } 
              else 
              {
                memmove(buf+pos+1,buf+pos,len-pos);
                buf[pos] = c;
                len++;
                pos++;
                buf[len] = '\0';
                refreshLine(prompt,buf,len,pos,cols);
              }
            }
            break;            
        }
    }

    return len;
}

int linenoise_getline( int id, char* buffer, int maxinput, const char* prompt )
{
  int count;
  
  if( history_max_lengths[ id ] == 0 )
  {
    fputs( prompt, stdout );
    fflush( stdout );
    return fgets( buffer, maxinput, stdin ) == NULL ? -1 : 0;
  }
  while( 1 )
  {
    count = linenoisePrompt( id, buffer, maxinput, prompt );
    if( count != -1 )
      printf( "\n" );
    if( count != LINENOISE_CTRL_C )
    {  
      if( count > 0 && buffer[ count ] != '\0' )
        buffer[ count ] = '\0';
      return count;
    }
  }
}

static int linenoise_internal_addhistory( int id, const char *line, int force_empty ) 
{
  char *linecopy;
  const char *p;
  const int history_max_len = history_max_lengths[ id ];
         
  if( history_max_len == 0 )
    return 0;
  if( histories[ id ] == NULL ) 
  {
    if( ( histories[ id ] = malloc( sizeof( char* ) * history_max_len ) ) == NULL )
    {
      fprintf( stderr, "out of memory in linenoise while trying to allocate history buffer.\n" );
      return 0;
    }
    memset( histories[ id ], 0, ( sizeof( char* ) * history_max_len ) );
  }
  while( 1 )
  {
    if( ( p = strchr( line, '\n' ) ) == NULL )
      p = line + strlen( line );
    if( p > line || force_empty == LINENOISE_PUSH_EMPTY )
    {
      if( ( linecopy = strndup( line, p - line ) ) == NULL )
      {
        fprintf( stderr, "out of memory in linenoise while trying to add a line to history.\n" );
        return 0; 
      }
      if( history_lengths[ id ] == history_max_len ) 
      {
        free( histories[ id ][ 0 ] );
        memmove( histories[ id ], histories[ id ] + 1, sizeof( char* ) * ( history_max_len - 1 ) );
        history_lengths[ id ] --;
      }
      histories[ id ][history_lengths[ id ]] = linecopy;
      history_lengths[ id ] ++;
    }
    if( *p == 0 )
      break;
    line = p + 1;
    if( *line == 0 )
      break;
  }
  return 1;
}

int linenoise_addhistory( int id, const char *line )
{
  return linenoise_internal_addhistory( id, line, LINENOISE_DONT_PUSH_EMPTY );
}

/* Save the history in the specified file. On success 0 is returned 
 * otherwise -1 is returned. */
int linenoise_savehistory( int id, const char *filename ) 
{  
  FILE *fp;
  int j;
  
  if( history_max_lengths[ id ] == 0 )
    return LINENOISE_HISTORY_NOT_ENABLED;  
  if( histories[ id ] == NULL || history_lengths[ id ] == 0 )
    return LINENOISE_HISTORY_EMPTY;
  if( ( fp = fopen( filename, "wb" ) ) == NULL )
    return -1;
  for( j = 0; j < history_lengths[ id ]; j++ )
    fprintf( fp, "%s\n", histories[ id ][ j ] );
  fclose( fp );
  return 0;
}

#else // #ifdef BUILD_LINENOISE

int linenoise_getline( int id, char* buffer, int maxinput, const char* prompt )
{
  ( void )id;
  fputs( prompt, stdout );
  fflush( stdout );
  return fgets( buffer, maxinput, stdin ) == NULL ? -1 : 0;
}

int linenoise_addhistory( int id, const char *line )
{
  return 0;
}

void linenoise_cleanup( int id )
{
}

int linenoise_savehistory( int id, const char *filename )
{
  return -1;
}

#endif // #ifdef BUILD_LINENOISE
