// Common code for all backends

#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "genstd.h"
#include "common.h"
#include "buf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "math.h"
#include "elua_adc.h"
#include "term.h"
#include "xmodem.h"
#include "elua_int.h"
#include "sermux.h"
#include "lua.h"
#include "lapi.h"
#include "lauxlib.h"

// [TODO] the new builder should automatically do this
#if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )
#define BUILD_INT_HANDLERS

#ifndef INT_TMR_MATCH
#define INT_TMR_MATCH         ELUA_INT_INVALID_INTERRUPT
#endif

extern const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ];

#endif // #if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )

// [TODO] the new builder should automatically do this
#ifndef VTMR_NUM_TIMERS
#define VTMR_NUM_TIMERS       0
#endif // #ifndef VTMR_NUM_TIMERS

// [TODO] the new builder should automatically do this
#ifndef CON_BUF_SIZE
#define CON_BUF_SIZE          0
#endif // #ifndef CON_BUF_SIZE

// [TODO] the new builder should automatically do this
#ifndef SERMUX_FLOW_TYPE
#define SERMUX_FLOW_TYPE      PLATFORM_UART_FLOW_NONE
#endif

// [TODO] the new builder should automatically do this
#ifndef CON_FLOW_TYPE
#define CON_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

// [TODO] the new builder should automatically do this
#ifndef CON_TIMER_ID
#define CON_TIMER_ID          PLATFORM_TIMER_SYS_ID
#endif

// ****************************************************************************
// XMODEM support code

#ifdef BUILD_XMODEM

static void xmodem_send( u8 data )
{
  platform_uart_send( CON_UART_ID, data );
}

static int xmodem_recv( timer_data_type timeout )
{
  return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, timeout );
}

#endif // #ifdef BUILD_XMODEM

// ****************************************************************************
// Terminal support code

#ifdef BUILD_TERM

#define TERM_TIMEOUT    100000 

static void term_out( u8 data )
{
  platform_uart_send( CON_UART_ID, data );
}

static int term_in( int mode )
{
  if( mode == TERM_INPUT_DONT_WAIT )
    return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, 0 );
  else
    return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, PLATFORM_TIMER_INF_TIMEOUT );
}

static int term_translate( int data )
{
  int c;
  
  if( isprint( data ) )
    return data;
  else if( data == 0x1B ) // escape sequence
  {
    // If we don't get a second char, we got a simple "ESC", so return KC_ESC
    // If we get a second char it must be '[', the next one is relevant for us
    if( platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT ) == -1 )
      return KC_ESC;
    if( ( c = platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT ) ) == -1 )
      return KC_UNKNOWN;
    if( c >= 0x41 && c <= 0x44 )
      switch( c )
      {
        case 0x41:
          return KC_UP;
        case 0x42:
          return KC_DOWN;
        case 0x43:
          return KC_RIGHT;
        case 0x44:
          return KC_LEFT;               
      }
    else if( c > 48 && c < 55 )
    {
      // Extended sequence: read another byte
      if( platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT ) != 126 )
        return KC_UNKNOWN;
      switch( c )
      {
        case 49:
          return KC_HOME;
        case 52:
          return KC_END;
        case 53:
          return KC_PAGEUP;
        case 54:
          return KC_PAGEDOWN;  
      }
    }
  }
  else if( data == 0x0D )
  {
    // CR/LF sequence, read the second char (LF) if applicable
    platform_uart_recv( CON_UART_ID, CON_TIMER_ID, TERM_TIMEOUT );
    return KC_ENTER;
  }
  else
  {
    switch( data )
    {
      case 0x09:
        return KC_TAB;
      case 0x7F:
//        return KC_DEL; // bogdanm: some terminal emulators (for example screen) return 0x7F for BACKSPACE :(
      case 0x08:
        return KC_BACKSPACE;
      case 26:
        return KC_CTRL_Z;
      case 1:
        return KC_CTRL_A;
      case 5:
        return KC_CTRL_E;
      case 3:
        return KC_CTRL_C;
      case 20:
        return KC_CTRL_T;
      case 21:
        return KC_CTRL_U;
      case 11:
        return KC_CTRL_K; 
    }
  }
  return KC_UNKNOWN;
}

#endif // #ifdef BUILD_TERM


// *****************************************************************************
// std functions and platform initialization

static void uart_send( int fd, char c )
{
  fd = fd;
  platform_uart_send( CON_UART_ID, c );
}

static int uart_recv( timer_data_type to )
{
  return platform_uart_recv( CON_UART_ID, CON_TIMER_ID, to );
}

void cmn_platform_init()
{
#ifdef BUILD_INT_HANDLERS
  platform_int_init();
#endif

#ifdef BUILD_SERMUX
  unsigned i;
  unsigned bufsizes[] = SERMUX_BUFFER_SIZES;  

  // Setup the serial multiplexer
  platform_uart_setup( SERMUX_PHYS_ID, SERMUX_PHYS_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  platform_uart_set_flow_control( SERMUX_PHYS_ID, SERMUX_FLOW_TYPE );
  cmn_uart_setup_sermux();

  // Set buffers for all virtual UARTs 
  for( i = 0; i < sizeof( bufsizes ) / sizeof( unsigned ); i ++ )
    platform_uart_set_buffer( i + SERMUX_SERVICE_ID_FIRST, bufsizes[ i ] );
#endif // #ifdef BUILD_SERMUX

#if defined( CON_UART_ID ) && CON_UART_ID < SERMUX_SERVICE_ID_FIRST && ( CON_UART_ID != CDC_UART_ID )
  // Setup console UART
  platform_uart_setup( CON_UART_ID, CON_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );  
  platform_uart_set_flow_control( CON_UART_ID, CON_FLOW_TYPE );
  platform_uart_set_buffer( CON_UART_ID, CON_BUF_SIZE );
#endif // #if defined( CON_UART_ID ) && CON_UART_ID < SERMUX_SERVICE_ID_FIRST

  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );  

#ifdef BUILD_XMODEM  
  // Initialize XMODEM
  xmodem_init( xmodem_send, xmodem_recv );    
#endif

#ifdef BUILD_TERM  
  // Initialize terminal
  term_init( TERM_LINES, TERM_COLS, term_out, term_in, term_translate );
#endif
}

// ****************************************************************************
// PIO functions

int platform_pio_has_port( unsigned port )
{
  return port < NUM_PIO;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%c", ( char )( port + PIO_PREFIX ) );
  return c;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
#if defined( PIO_PINS_PER_PORT )
  return port < NUM_PIO && pin < PIO_PINS_PER_PORT;
#elif defined( PIO_PIN_ARRAY )
  const u8 pio_port_pins[] = PIO_PIN_ARRAY;
  return port < NUM_PIO && pin < pio_port_pins[ port ];
#else
  #error "You must define either PIO_PINS_PER_PORT of PIO_PIN_ARRAY in platform_conf.h"
#endif
}

// ****************************************************************************
// CAN functions

int platform_can_exists( unsigned id )
{
  return id < NUM_CAN;
}

// ****************************************************************************
// SPI functions


int platform_spi_exists( unsigned id )
{
  return id < NUM_SPI;
}

// ****************************************************************************
// PWM functions

int platform_pwm_exists( unsigned id )
{
  return id < NUM_PWM;
}

// ****************************************************************************
// CPU functions

u32 platform_cpu_get_frequency()
{
  return CPU_FREQUENCY;
}

// ****************************************************************************
// ADC functions

int platform_adc_exists( unsigned id )
{
  return id < NUM_ADC;
}

#ifdef BUILD_ADC

u32 platform_adc_get_maxval( unsigned id )
{
  return pow( 2, ADC_BIT_RESOLUTION ) - 1;
}

u32 platform_adc_set_smoothing( unsigned id, u32 length )
{
  return adc_update_smoothing( id, ( u8 )intlog2( ( unsigned ) length ) );
}

void platform_adc_set_blocking( unsigned id, u32 mode )
{
  adc_get_ch_state( id )->blocking = mode;
}

void platform_adc_set_freerunning( unsigned id, u32 mode )
{
  adc_get_ch_state( id )->freerunning = mode;
}

u32 platform_adc_is_done( unsigned id )
{
  return adc_get_ch_state( id )->op_pending == 0;
}

void platform_adc_set_timer( unsigned id, u32 timer )
{
  elua_adc_dev_state *d = adc_get_dev_state( 0 );

  if ( d->timer_id != timer )
    d->running = 0;
  platform_adc_stop( id );
  d->timer_id = timer;
}

#endif // #ifdef BUILD_ADC

// ****************************************************************************
// Allocator support

#define MIN_ALIGN         8
#define MIN_ALIGN_SHIFT   3

extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  void* mstart[] = MEM_START_ADDRESS;
  u32 p;

  if( id >= sizeof( mstart ) / sizeof( void* ) )
    return NULL;
  p = ( u32 )mstart[ id ];
  if( p & ( MIN_ALIGN - 1 ) )
    p = ( ( p >> MIN_ALIGN_SHIFT ) + 1 ) << MIN_ALIGN_SHIFT;
  return ( void* )p;
}

void* platform_get_last_free_ram( unsigned id )
{
  void* mend[] = MEM_END_ADDRESS;
  u32 p;

  if( id >= sizeof( mend ) / sizeof( void* ) )
    return NULL;
  p = ( u32 )mend[ id ];
  if( p & ( MIN_ALIGN - 1 ) )
    p = ( ( p >> MIN_ALIGN_SHIFT ) - 1 ) << MIN_ALIGN_SHIFT;
  return ( void* )p;
}

// I2C support
int platform_i2c_exists( unsigned id )
{
#ifndef NUM_I2C
  return 0;
#else
  return id < NUM_I2C;
#endif
}

// ****************************************************************************
// Interrupt support
#ifdef BUILD_INT_HANDLERS

int platform_cpu_set_interrupt( elua_int_id id, elua_int_resnum resnum, int status )
{
  elua_int_p_set_status ps;

  if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
    return PLATFORM_INT_INVALID;
  if( ( ps = elua_int_table[ id - ELUA_INT_FIRST_ID ].int_set_status ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  if( id == INT_TMR_MATCH )
    return cmn_tmr_int_set_status( resnum, status );
  return ps( resnum, status );
}

int platform_cpu_get_interrupt( elua_int_id id, elua_int_resnum resnum )
{
  elua_int_p_get_status pg;

  if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
    return PLATFORM_INT_INVALID;
  if( ( pg = elua_int_table[ id - ELUA_INT_FIRST_ID ].int_get_status ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  if( id == INT_TMR_MATCH )
    return cmn_tmr_int_get_status( resnum );
  return pg( resnum );
}

int platform_cpu_get_interrupt_flag( elua_int_id id, elua_int_resnum resnum, int clear )
{
  elua_int_p_get_flag pf;

  if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
    return PLATFORM_INT_INVALID;
  if( ( pf = elua_int_table[ id - ELUA_INT_FIRST_ID ].int_get_flag ) == NULL )
    return PLATFORM_INT_NOT_HANDLED;
  if( id == INT_TMR_MATCH )
    return cmn_tmr_int_get_flag( resnum, clear );
  return pf( resnum, clear );
}

// Common interrupt handling
void cmn_int_handler( elua_int_id id, elua_int_resnum resnum )
{
  elua_int_add( id, resnum );
#ifdef BUILD_C_INT_HANDLERS
  elua_int_c_handler phnd = elua_int_get_c_handler( id );
  if( phnd )
    phnd( resnum );
#endif
}

#endif // #ifdef BUILD_INT_HANDLERS

// ****************************************************************************
// Internal flash support functions (currently used only by WOFS)

#if defined( BUILD_WOFS ) && !defined( ELUA_CPU_LINUX )

// This symbol must be exported by the linker command file and must reflect the
// TOTAL size of flash used by the eLua image (not only the code and constants,
// but also .data and whatever else ends up in the eLua image). WOFS will start
// at the next usable (aligned to a flash sector boundary) address after 
// flash_used_size.
extern char flash_used_size[];

// Helper function: find the flash sector in which an address resides
// Return the sector number, as well as the start and end address of the sector
static u32 flashh_find_sector( u32 address, u32 *pstart, u32 *pend )
{
  address -= INTERNAL_FLASH_START_ADDRESS;
#ifdef INTERNAL_FLASH_SECTOR_SIZE
  // All the sectors in the flash have the same size, so just align the address
  u32 sect_id = address / INTERNAL_FLASH_SECTOR_SIZE;

  if( pstart )
    *pstart = sect_id * INTERNAL_FLASH_SECTOR_SIZE + INTERNAL_FLASH_START_ADDRESS;
  if( pend )
    *pend = ( sect_id + 1 ) * INTERNAL_FLASH_SECTOR_SIZE + INTERNAL_FLASH_START_ADDRESS - 1;
  return sect_id;
#else // #ifdef INTERNAL_FLASH_SECTOR_SIZE
  // The flash has blocks of different size
  // Their size is decribed in the INTERNAL_FLASH_SECTOR_ARRAY macro
  const u32 flash_sect_size[] = INTERNAL_FLASH_SECTOR_ARRAY;
  u32 total = 0, i = 0;

  while( ( total <= address ) && ( i < sizeof( flash_sect_size ) / sizeof( u32 ) ) )
    total += flash_sect_size[ i ++ ];
  if( pstart )
    *pstart = ( total - flash_sect_size[ i - 1 ] ) + INTERNAL_FLASH_START_ADDRESS;
  if( pend )
    *pend = total + INTERNAL_FLASH_START_ADDRESS - 1;
  return i - 1;
#endif // #ifdef INTERNAL_FLASH_SECTOR_SIZE
}

u32 platform_flash_get_sector_of_address( u32 addr )
{
  return flashh_find_sector( addr, NULL, NULL );
}

u32 platform_flash_get_num_sectors()
{
#ifdef INTERNAL_FLASH_SECTOR_SIZE
  return INTERNAL_FLASH_SIZE / INTERNAL_FLASH_SECTOR_SIZE;
#else // #ifdef INTERNAL_FLASH_SECTOR_SIZE
  const u32 flash_sect_size[] = INTERNAL_FLASH_SECTOR_ARRAY;

  return sizeof( flash_sect_size ) / sizeof( u32 );
#endif // #ifdef INTERNAL_FLASH_SECTOR_SIZE
}

u32 platform_flash_get_first_free_block_address( u32 *psect )
{
  // Round the total used flash size to the closest flash block address
  u32 temp, sect;

  sect = flashh_find_sector( ( u32 )flash_used_size + INTERNAL_FLASH_START_ADDRESS - 1, NULL, &temp );
  if( psect )
    *psect = sect + 1;
  return temp + 1;
}

u32 platform_flash_write( const void *from, u32 toaddr, u32 size )
{
#ifndef INTERNAL_FLASH_WRITE_UNIT_SIZE
  return platform_s_flash_write( from, toaddr, size );
#else // #ifindef INTERNAL_FLASH_WRITE_UNIT_SIZE
  u32 temp, rest, ssize = size;
  unsigned i;
  char tmpdata[ INTERNAL_FLASH_WRITE_UNIT_SIZE ];
  const u8 *pfrom = ( const u8* )from;
  const u32 blksize = INTERNAL_FLASH_WRITE_UNIT_SIZE;
  const u32 blkmask = INTERNAL_FLASH_WRITE_UNIT_SIZE - 1;

  // Align the start
  if( toaddr & blkmask )
  {
    rest = toaddr & blkmask;
    temp = toaddr & ~blkmask; // this is the actual aligned address
    memcpy( tmpdata, ( const void* )temp, blksize );
    for( i = rest; size && ( i < blksize ); i ++, size --, pfrom ++ )
      tmpdata[ i ] = *pfrom;
    platform_s_flash_write( tmpdata, temp, blksize );
    if( size == 0 )
      return ssize;
    toaddr = temp + blksize;
  }
  // The start address is now a multiple of blksize
  // Compute how many bytes we can write as multiples of blksize
  rest = size & blkmask;
  temp = size & ~blkmask;
  // Program the blocks now
  if( temp )
  {
    platform_s_flash_write( pfrom, toaddr, temp );
    toaddr += temp;
    pfrom += temp;
  }
  // And the final part of a block if needed
  if( rest )
  {
    memcpy( tmpdata, ( const void* )toaddr, blksize );
    for( i = 0; size && ( i < rest ); i ++, size --, pfrom ++ )
      tmpdata[ i ] = *pfrom;
    platform_s_flash_write( tmpdata, toaddr, blksize );
  }
  return ssize;
#endif // #ifndef INTERNAL_FLASH_WRITE_UNIT_SIZE
}

#endif // #ifdef BUILD_WOFS

// ****************************************************************************
// Misc support

unsigned int intlog2( unsigned int v )
{
  unsigned r = 0;

  while (v >>= 1)
  {
    r++;
  }
  return r;
}

// 64-bits integer printf support seems to be broken in some versions of Newlib...
const char* cmn_str64( u64 x )
{
  static char nr[ 32 ];
  u64 q, r;
  unsigned l = 30;

  memset( nr, 0, 32 );
  do
  {
    q = x / 10;
    r = x % 10;
    nr[ l -- ] = r + '0';
    x = q;
  } while( x != 0 );
  return nr + l + 1;
}

// Read a timeout spec from the user and return it
// The timeout spec has the format [timer_id, timeout]. Both arguments are optional: 
// If none is specified -> defaults to infinite timeout
// If timer_id is specified, but timeout is not specified -> defaults to infinite timeout
// If timeout is PLATFORM_TIMER_INF_TIMEOUT -> also infinite timeout (independent of timer_id)
// If both are specified -> wait the specified timeout on the specified timer_id
// If timer_id is 'nil' the system timer will be used
void cmn_get_timeout_data( lua_State *L, int pidx, unsigned *pid, timer_data_type *ptimeout )
{
  lua_Number tempn;

  *ptimeout = PLATFORM_TIMER_INF_TIMEOUT; 
  *pid = ( unsigned )luaL_optinteger( L, pidx, PLATFORM_TIMER_SYS_ID );
  if( lua_type( L, pidx + 1 ) == LUA_TNUMBER )
  {
    tempn = lua_tonumber( L, pidx + 1 );
    if( tempn < 0 || tempn > PLATFORM_TIMER_INF_TIMEOUT )
      luaL_error( L, "invalid timeout value" );
    *ptimeout = ( timer_data_type )tempn;
  }
  if( *pid == PLATFORM_TIMER_SYS_ID && !platform_timer_sys_available() )
    luaL_error( L, "the system timer is not implemented on this platform" );
}

