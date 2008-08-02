// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

// Platform specific includes
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"
#include "usart.h"
#include "ssi.h"
#include "timer.h"

// *****************************************************************************
// std function
static void uart_send( int fd, char c )
{
  fd = fd;
  UARTCharPut( UART0_BASE, c );
}

static int uart_recv()
{
  return UARTCharGet( UART0_BASE );
}

// ****************************************************************************
// Platform initialization

static u32 timer_base[] = { TIMER0_BASE, TIMER1_BASE, TIMER2_BASE, TIMER3_BASE };

int platform_init()
{ 
  unsigned i;
  
  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
  
  // Enable peripherals
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);           
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);             
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);             
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);             
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);             
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);             
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);             
  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

  // Configure the UART for 115,200, 8-N-1 operation.
  // (this also enables the UART)
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);  
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE)); 
  
  // Setup timers
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);  
  for( i = 0; i < 4; i ++ )
  {
    TimerConfigure(timer_base[ i ], TIMER_CFG_32_BIT_PER);
    TimerEnable(timer_base[ i ], TIMER_A);
  }
                         
  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );      
  
  // All done
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO

int platform_pio_has_port( unsigned port )
{
  return port <= 6;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
  if( port <= 3 )
    return pin <= 7;
  else if( ( port == 4 ) || ( port == 5 ) )
    return pin <= 3;
  else if( port == 6 )
    return pin <= 1;
  return 0;
}

static const pio_type port_base[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE,
                                      GPIO_PORTF_BASE, GPIO_PORTG_BASE };

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 0, base = port_base[ port ];
  
  switch( op )
  {
    case PLATFORM_IO_PORT_SET_VALUE:    
      GPIOPinWrite( base, 0xFF, pinmask );
      break;
      
    case PLATFORM_IO_PIN_SET:
      GPIOPinWrite( base, pinmask, pinmask );
      break;
      
    case PLATFORM_IO_PIN_CLEAR:
      GPIOPinWrite( base, pinmask, 0 );
      break;
      
    case PLATFORM_IO_PORT_DIR_INPUT:
      pinmask = 0xFF;      
    case PLATFORM_IO_PIN_DIR_INPUT:
      GPIOPinTypeGPIOInput( base, pinmask );
      break;
      
    case PLATFORM_IO_PORT_DIR_OUTPUT:      
      pinmask = 0xFF;
    case PLATFORM_IO_PIN_DIR_OUTPUT:
      GPIOPinTypeGPIOOutput( base, pinmask );
      break;      
            
    case PLATFORM_IO_PORT_GET_VALUE:
      retval = GPIOPinRead( base, 0xFF );
      break;
      
    case PLATFORM_IO_PIN_GET:
      retval = GPIOPinRead( base, pinmask ) ? 1 : 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// SPI

#define SSI_CLK             GPIO_PIN_2
#define SSI_TX              GPIO_PIN_5
#define SSI_RX              GPIO_PIN_4

int platform_spi_exists( unsigned id )
{
  return id < 1;
}

u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits )
{
  unsigned protocol;
  
  id = id;
  if( cpol == 0 )
    protocol = cpha ? SSI_FRF_MOTO_MODE_1 : SSI_FRF_MOTO_MODE_0;
  else
    protocol = cpha ? SSI_FRF_MOTO_MODE_3 : SSI_FRF_MOTO_MODE_2;
  mode = mode == PLATFORM_SPI_MASTER ? SSI_MODE_MASTER : SSI_MODE_SLAVE;  
  SSIDisable( SSI0_BASE );
  GPIOPinTypeSSI(GPIO_PORTA_BASE, SSI_CLK | SSI_TX | SSI_RX);
  GPIOPadConfigSet(GPIO_PORTA_BASE, SSI_CLK, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);    
  SSIConfigSetExpClk( SSI0_BASE, SysCtlClockGet(), protocol, mode, clock, databits );
  SSIEnable( SSI0_BASE );
  return clock;
}

spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data )
{
  id = id;
  SSIDataPut( SSI0_BASE, data );
  SSIDataGet( SSI0_BASE, &data );
  return data;
}

void platform_spi_select( unsigned id, int is_select )
{
  // This platform doesn't have a hardware SS pin, so there's nothing to do here  
  id = id;
  is_select = is_select;
}

// ****************************************************************************
// UART

int platform_uart_exists( unsigned id )
{
  return id <= 1;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 config;
  
  if( id == 0 )
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  else
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);    
  switch( databits )
  {
    case 5:
      config = UART_CONFIG_WLEN_5;
      break;
    case 6:
      config = UART_CONFIG_WLEN_6;
      break;
    case 7:
      config = UART_CONFIG_WLEN_7;
      break;
    default:
      config = UART_CONFIG_WLEN_8;
      break;
  }
  config |= ( stopbits == PLATFORM_UART_STOPBITS_1 ) ? UART_CONFIG_STOP_ONE : UART_CONFIG_STOP_TWO;
  if( parity == PLATFORM_UART_PARITY_EVEN )
    config |= UART_CONFIG_PAR_EVEN;
  else if( parity == PLATFORM_UART_PARITY_ODD )
    config |= UART_CONFIG_PAR_ODD;
  else
    config |= UART_CONFIG_PAR_NONE;
  return UARTConfigSetExpClk(id == 0 ? UART0_BASE : UART1_BASE, SysCtlClockGet(), baud, config);
}

void platform_uart_send( unsigned id, u8 data )
{
  UARTCharPut( id == 0 ? UART0_BASE : UART1_BASE, data );
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  u32 base = id == 0 ? UART0_BASE : UART1_BASE;
  timer_data_type tmr_start, tmr_crt;
  int res;
  
  if( timeout == 0 )
  {
    return UARTCharGetNonBlocking( base );  
  }
  else if( timeout == PLATFORM_UART_INFINITE_TIMEOUT )
  {
    // Receive char blocking
    return UARTCharGet( base );
  }
  else
  {
    // Receive char with the specified timeout
    tmr_start = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
    while( 1 )
    {
      if( ( res = UARTCharGetNonBlocking( base ) ) >= 0 )
        break;
      tmr_crt = platform_timer_op( timer_id, PLATFORM_TIMER_OP_READ, 0 );
      if( platform_timer_get_diff_us( timer_id, tmr_crt, tmr_start ) >= timeout )
        break;
    }
    return res;
  }
}

// ****************************************************************************
// Timer

int platform_timer_exists( unsigned id )
{
  return id <= 3;
}

void platform_timer_delay( unsigned id, u32 delay_us )
{
  timer_data_type final;
  u32 base = timer_base[ id ];
  
  final = 0xFFFFFFFF - ( ( ( u64 )delay_us * SysCtlClockGet() ) / 1000000 );
  TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
  while( TimerValueGet( base, TIMER_A ) > final );
}
      
u32 platform_timer_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  u32 base = timer_base[ id ]; 
  
  data = data;
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
      res = 0xFFFFFFFF;
      TimerLoadSet( base, TIMER_A, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_READ:
      res = TimerValueGet( base, TIMER_A );
      break;
      
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
      res = platform_timer_get_diff_us( id, 0, 0xFFFFFFFF );
      break;
      
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
      res = platform_timer_get_diff_us( id, 0, 1 );
      break;
      
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      res = SysCtlClockGet();
      break;
      
  }
  return res;
}

u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start )
{
  timer_data_type temp;
  
  id = id;
  if( start < end )
  {
    temp = end;
    end = start;
    start = temp;
  }
  return ( ( u64 )( start - end ) * 1000000 ) / SysCtlClockGet();
}

// ****************************************************************************
// Platform data

const char* platform_pd_get_name()
{
  return "LM3S8962";
}

u32 platform_pd_get_cpu_frequency()
{
  return SysCtlClockGet();
}

// ****************************************************************************
// Allocator support

extern char end[];

void* platform_get_first_free_ram()
{
  return ( void* )end;
}

#define STACK_SIZE 256
#define SRAM_SIZE ( 64 * 1024 )
void* platform_get_last_free_ram()
{
  return ( void* )( SRAM_BASE + SRAM_SIZE - STACK_SIZE );
}
