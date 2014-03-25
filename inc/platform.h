// Platform-specific functions

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "devman.h"
#include "elua_int.h"

// Error / status codes
enum
{
  PLATFORM_ERR,
  PLATFORM_OK,
  PLATFORM_UNDERFLOW = -1
};

// Platform initialization
int platform_init(void);
void platform_int_init(void);

// *****************************************************************************
// PIO subsection
// "Virtual ports": 16 ports (PA...PF), 32-bits each for a total of 512 I/O pins.
// They are coded within a single integer, where the high part encodes the port
// number, while the lower part encodes the pin number

typedef u32 pio_type;
typedef u32 pio_code;
#define PLATFORM_IO_PORTS                     16
#define PLATFORM_IO_PORTS_BITS                4
#define PLATFORM_IO_PINS                      32
#define PLATFORM_IO_PINS_BITS                 5
#define PLATFORM_IO_FULL_PORT_BIT             14
#define PLATFORM_IO_FULL_PORT_MASK            ( 1 << PLATFORM_IO_FULL_PORT_BIT )
#define PLATFORM_IO_ENCODE( port, pin, full ) ( ( ( port ) << PLATFORM_IO_PINS_BITS ) | ( pin ) | ( ( full ) ? PLATFORM_IO_FULL_PORT_MASK : 0 ) )
#define PLATFORM_IO_GET_PORT( code )          ( ( ( code ) >> PLATFORM_IO_PINS_BITS ) & ( ( 1 << PLATFORM_IO_PORTS_BITS ) - 1 ) )
#define PLATFORM_IO_GET_PIN( code )           ( ( code ) & ( ( 1 << PLATFORM_IO_PINS_BITS ) - 1 ) )
#define PLATFORM_IO_IS_PORT( code )           ( ( ( code ) & PLATFORM_IO_FULL_PORT_MASK ) != 0 )
#define PLATFORM_IO_ALL_PINS                  0xFFFFFFFFUL
#define PLATFORM_IO_ENC_PORT                  1
#define PLATFORM_IO_ENC_PIN                   0

#define PLATFORM_IO_READ_IN_MASK              0
#define PLATFORM_IO_READ_OUT_MASK             1

enum
{
  // Pin operations
  PLATFORM_IO_PIN_SET,
  PLATFORM_IO_PIN_CLEAR,
  PLATFORM_IO_PIN_GET,
  PLATFORM_IO_PIN_DIR_INPUT,
  PLATFORM_IO_PIN_DIR_OUTPUT,
  PLATFORM_IO_PIN_PULLUP,
  PLATFORM_IO_PIN_PULLDOWN,
  PLATFORM_IO_PIN_NOPULL,
  // Port operations
  PLATFORM_IO_PORT_SET_VALUE,
  PLATFORM_IO_PORT_GET_VALUE,
  PLATFORM_IO_PORT_DIR_INPUT,
  PLATFORM_IO_PORT_DIR_OUTPUT
};

// The platform I/O functions
int platform_pio_has_port( unsigned port );
const char* platform_pio_get_prefix( unsigned port );
int platform_pio_has_pin( unsigned port, unsigned pin );
int platform_pio_get_num_pins( unsigned port );
pio_type platform_pio_op( unsigned port, pio_type pinmask, int op );

// *****************************************************************************
// Timer subsection

// The ID of the system timer
#define PLATFORM_TIMER_SYS_ID                 0x100

#if defined( LUA_NUMBER_INTEGRAL ) && !defined( LUA_INTEGRAL_LONGLONG )
// Maximum values of the system timer
#define PLATFORM_TIMER_SYS_MAX                ( ( 1LL << 32 ) - 2 )
// Timer data type
typedef u32 timer_data_type;
#else
// Maximum values of the system timer
#define PLATFORM_TIMER_SYS_MAX                ( ( 1LL << 52 ) - 2 )
// Timer data type
typedef u64 timer_data_type;
#endif // #if defined( LUA_NUMBER_INTEGRAL ) && !defined( LUA_INTEGRAL_LONGLONG )

// This constant means 'infinite timeout'
#define PLATFORM_TIMER_INF_TIMEOUT            ( PLATFORM_TIMER_SYS_MAX + 1 )

// System timer frequency
#define PLATFORM_TIMER_SYS_FREQ               1000000

// Interrupt types
#define PLATFORM_TIMER_INT_ONESHOT            1
#define PLATFORM_TIMER_INT_CYCLIC             2

// Match interrupt error codes
#define PLATFORM_TIMER_INT_OK                 0
#define PLATFORM_TIMER_INT_TOO_SHORT          1
#define PLATFORM_TIMER_INT_TOO_LONG           2
#define PLATFORM_TIMER_INT_INVALID_ID         3

// Timer operations
enum
{
  PLATFORM_TIMER_OP_START,
  PLATFORM_TIMER_OP_READ,
  PLATFORM_TIMER_OP_SET_CLOCK,
  PLATFORM_TIMER_OP_GET_CLOCK,
  PLATFORM_TIMER_OP_GET_MAX_DELAY,
  PLATFORM_TIMER_OP_GET_MIN_DELAY,
  PLATFORM_TIMER_OP_GET_MAX_CNT
};

// The platform timer functions
int platform_timer_exists( unsigned id );
void platform_timer_delay( unsigned id, timer_data_type delay_us );
void platform_s_timer_delay( unsigned id, timer_data_type delay_us );
timer_data_type platform_timer_op( unsigned id, int op, timer_data_type data );
timer_data_type platform_s_timer_op( unsigned id, int op, timer_data_type data );
int platform_timer_set_match_int( unsigned id, timer_data_type period_us, int type );
int platform_s_timer_set_match_int( unsigned id, timer_data_type period_us, int type );
timer_data_type platform_timer_get_diff_us( unsigned id, timer_data_type start, timer_data_type end );
// System timer functions
timer_data_type platform_timer_read_sys(void);
int platform_timer_sys_available(void);
// The next 3 functions need to be implemented only if the generic system timer mechanism
// (src/common.c:cmn_systimer*) is used by the backend
u64 platform_timer_sys_raw_read(void);
void platform_timer_sys_enable_int(void);
void platform_timer_sys_disable_int(void);

// Convenience macros
#define platform_timer_read( id )             platform_timer_op( id, PLATFORM_TIMER_OP_READ, 0 )
#define platform_timer_start( id )            platform_timer_op( id, PLATFORM_TIMER_OP_START, 0 )
#define platform_timer_get_diff_crt( id, v )  platform_timer_get_diff_us( id, v, platform_timer_read( id ) )
#define platform_timer_sys_delay( us )        platform_timer_delay( PLATFORM_TIMER_SYS_ID, us )
#define platform_timer_get_max_cnt( id )      platform_timer_op( id, PLATFORM_TIMER_OP_GET_MAX_CNT, 0 )

// *****************************************************************************
// CAN subsection

// Maximum length for any CAN message
#define PLATFORM_CAN_MAXLEN                   8

// eLua CAN ID types
enum
{
  ELUA_CAN_ID_STD = 0,
  ELUA_CAN_ID_EXT
};

int platform_can_exists( unsigned id );
u32 platform_can_setup( unsigned id, u32 clock );
int platform_can_send( unsigned id, u32 canid, u8 idtype, u8 len, const u8 *data );
int platform_can_recv( unsigned id, u32 *canid, u8 *idtype, u8 *len, u8 *data );

// *****************************************************************************
// SPI subsection

// There are 4 "virtual" SPI ports (SPI0...SPI3).
#define PLATFORM_SPI_TOTAL                    4
// TODO: PLATFORM_SPI_TOTAL is not used - figure out purpose, or remove?

// SPI mode
#define PLATFORM_SPI_MASTER                   1
#define PLATFORM_SPI_SLAVE                    0
// SS values
#define PLATFORM_SPI_SELECT_ON                1
#define PLATFORM_SPI_SELECT_OFF               0
// SPI enable/disable
#define PLATFORM_SPI_ENABLE                   1
#define PLATFORM_SPI_DISABLE                  0

// Data types
typedef u32 spi_data_type;

// The platform SPI functions
int platform_spi_exists( unsigned id );
u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits );
spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data );
void platform_spi_select( unsigned id, int is_select );

// *****************************************************************************
// UART subsection

// There are 4 "virtual" UART ports (UART0...UART3).
#define PLATFORM_UART_TOTAL                   4
// TODO: PLATFORM_UART_TOTAL is not used - figure out purpose, or remove?
// Note: Some CPUs (e.g. LM4F/TM4C) have more than 4 hardware UARTs

// Pseudo ID of UART over CDC
#define CDC_UART_ID     0xB0

// Parity
enum
{
  PLATFORM_UART_PARITY_EVEN,
  PLATFORM_UART_PARITY_ODD,
  PLATFORM_UART_PARITY_NONE,
  PLATFORM_UART_PARITY_MARK,
  PLATFORM_UART_PARITY_SPACE
};

// Stop bits
enum
{
  PLATFORM_UART_STOPBITS_1,
  PLATFORM_UART_STOPBITS_1_5,
  PLATFORM_UART_STOPBITS_2
};

// Flow control types (this is a bit mask, one can specify PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS )
#define PLATFORM_UART_FLOW_NONE               0
#define PLATFORM_UART_FLOW_RTS                1
#define PLATFORM_UART_FLOW_CTS                2

// The platform UART functions
int platform_uart_exists( unsigned id );
u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits );
int platform_uart_set_buffer( unsigned id, unsigned size );
void platform_uart_send( unsigned id, u8 data );
void platform_s_uart_send( unsigned id, u8 data );
int platform_uart_recv( unsigned id, unsigned timer_id, timer_data_type timeout );
int platform_s_uart_recv( unsigned id, timer_data_type timeout );
int platform_uart_set_flow_control( unsigned id, int type );
int platform_s_uart_set_flow_control( unsigned id, int type );

// *****************************************************************************
// PWM subsection

// There are 16 "virtual" PWM channels (PWM0...PWM15)
#define PLATFORM_PWM_TOTAL                    16
// TODO: PLATFORM_PWM_TOTAL is not used - figure out purpose, or remove?

// The platform PWM functions
int platform_pwm_exists( unsigned id );
u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty );
void platform_pwm_start( unsigned id );
void platform_pwm_stop( unsigned id );
u32 platform_pwm_set_clock( unsigned id, u32 data );
u32 platform_pwm_get_clock( unsigned id );

// *****************************************************************************
// CPU specific functions

#define PLATFORM_CPU_DISABLE            0
#define PLATFORM_CPU_ENABLE             1

// Interrupt functions return status
#define PLATFORM_INT_OK                 0
#define PLATFORM_INT_GENERIC_ERROR      ( -1 )
#define PLATFORM_INT_INVALID            ( -2 )
#define PLATFORM_INT_NOT_HANDLED        ( -3 )
#define PLATFORM_INT_BAD_RESNUM         ( -4 )

int platform_cpu_set_global_interrupts( int status );
int platform_cpu_get_global_interrupts(void);
int platform_cpu_set_interrupt( elua_int_id id, elua_int_resnum resnum, int status );
int platform_cpu_get_interrupt( elua_int_id id, elua_int_resnum resnum );
int platform_cpu_get_interrupt_flag( elua_int_id id, elua_int_resnum resnum, int clear );
u32 platform_cpu_get_frequency(void);

// *****************************************************************************
// The platform ADC functions

// Functions requiring platform-specific implementation
int  platform_adc_update_sequence(void);
int  platform_adc_start_sequence(void);
void platform_adc_stop( unsigned id );
u32  platform_adc_set_clock( unsigned id, u32 frequency);
int  platform_adc_check_timer_id( unsigned id, unsigned timer_id );

// ADC Common Functions
int  platform_adc_exists( unsigned id );
u32  platform_adc_get_maxval( unsigned id );
u32  platform_adc_set_smoothing( unsigned id, u32 length );
void platform_adc_set_blocking( unsigned id, u32 mode );
void platform_adc_set_freerunning( unsigned id, u32 mode );
u32  platform_adc_is_done( unsigned id );
void platform_adc_set_timer( unsigned id, u32 timer );

// *****************************************************************************
// I2C platform interface

// I2C speed
enum
{
  PLATFORM_I2C_SPEED_SLOW = 100000,
  PLATFORM_I2C_SPEED_FAST = 400000
};

// I2C direction
enum
{
  PLATFORM_I2C_DIRECTION_TRANSMITTER,
  PLATFORM_I2C_DIRECTION_RECEIVER
};

int platform_i2c_exists( unsigned id );
u32 platform_i2c_setup( unsigned id, u32 speed );
void platform_i2c_send_start( unsigned id );
void platform_i2c_send_stop( unsigned id );
int platform_i2c_send_address( unsigned id, u16 address, int direction );
int platform_i2c_send_byte( unsigned id, u8 data );
int platform_i2c_recv_byte( unsigned id, int ack );

// *****************************************************************************
// Ethernet specific functions

void platform_eth_send_packet( const void* src, u32 size );
u32 platform_eth_get_packet_nb( void* buf, u32 maxlen );
void platform_eth_force_interrupt(void);
u32 platform_eth_get_elapsed_time(void);

// *****************************************************************************
// Internal flash erase/write functions
// Currently used by WOFS

u32 platform_flash_get_first_free_block_address( u32 *psect );
u32 platform_flash_get_sector_of_address( u32 addr );
u32 platform_flash_write( const void *from, u32 toaddr, u32 size );
u32 platform_s_flash_write( const void *from, u32 toaddr, u32 size );
u32 platform_flash_get_num_sectors(void);
int platform_flash_erase_sector( u32 sector_id );

// *****************************************************************************
// Allocator support

void* platform_get_first_free_ram( unsigned id );
void* platform_get_last_free_ram( unsigned id );

#endif
