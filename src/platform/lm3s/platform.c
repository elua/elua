// Platform-dependent functions

#include "platform.h"
#include "type.h"
#include "devman.h"
#include "genstd.h"
#include <reent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "uip_arp.h"
#include "elua_uip.h" 
#include "uip-conf.h"
#include "platform_conf.h"

// Platform specific includes
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_pwm.h"
#include "hw_nvic.h"
#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"
#include "usart.h"
#include "ssi.h"
#include "timer.h"
#include "pwm.h"
#include "utils.h"
#include "ethernet.h"
#include "systick.h"
#include "flash.h"
#include "interrupt.h"
#include "elua_net.h"
#include "dhcpc.h"

// UIP sys tick data
#define SYSTICKHZ               4
#define SYSTICKMS               (1000 / SYSTICKHZ)

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

// forward
static void timers_init();
static void uarts_init();
static void spis_init();
static void pios_init();
static void pwms_init();
static void eth_init();

int platform_init()
{ 
  // Set the clocking to run from PLL
  SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
  
  // Setup PIO
  pios_init();

  // Setup SSIs
  spis_init();

  // Setup UARTs
  uarts_init();
  
  // Setup timers
  timers_init();        
  
  // Setup PWMs
  pwms_init();                 
  
  // Setup ethernet (TCP/IP)
  eth_init();

  // Set the send/recv functions                          
  std_set_send_func( uart_send );
  std_set_get_func( uart_recv );      
  
  // All done
  return PLATFORM_OK;
} 

// ****************************************************************************
// PIO
// Same configuration on LM3S8962 and LM3S6965

static const u32 pio_base[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE, 
                                GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTH_BASE };
static const u32 pio_sysctl[] = { SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD, 
                                  SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOG, SYSCTL_PERIPH_GPIOH };
static const u8 pio_port_pins[] = { 8, 8, 8, 8, 4, 4, 2, 0 };
#define PIOS_COUNT 42
#define PIOS_PORT_COUNT 7

static void pios_init()
{
  unsigned i;

  for( i = 0; i < PIOS_PORT_COUNT; i ++ )
  {
    SysCtlPeripheralEnable(pio_sysctl[ i ]);
  }
}

int platform_pio_has_port( unsigned port )
{
  return port < PIOS_PORT_COUNT;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%c", ( char )( port + 'A' ) );
  return c;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
  return pin < pio_port_pins[ port ];
}

pio_type platform_pio_op( unsigned port, pio_type pinmask, int op )
{
  pio_type retval = 1, base = pio_base[ port ];
  
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
      
    case PLATFORM_IO_PIN_PULLUP:
    case PLATFORM_IO_PIN_PULLDOWN:
      GPIOPadConfigSet( base, pinmask, GPIO_STRENGTH_8MA, op == PLATFORM_IO_PIN_PULLUP ? GPIO_PIN_TYPE_STD_WPU : GPIO_PIN_TYPE_STD_WPD );
      break;
      
    case PLATFORM_IO_PIN_NOPULL:
      GPIOPadConfigSet( base, pinmask, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD );
      break;
      
    default:
      retval = 0;
      break;
  }
  return retval;
}

// ****************************************************************************
// SPI
// Same configuration on LM3S8962 and LM3S6965

// All possible LM3S SPIs defs
// FIXME this anticipates support for a platform with 2 SPI port
//  PIN info extracted from LM3S6950 and 5769 datasheets
static const u32 spi_base[] = { SSI0_BASE, SSI1_BASE };
static const u32 spi_sysctl[] = { SYSCTL_PERIPH_SSI0, SYSCTL_PERIPH_SSI1 };
static const u32 spi_gpio_base[] = { GPIO_PORTA_BASE | GPIO_PORTE_BASE };
static const u8 spi_gpio_pins[] = { GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
                                    GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 };
//                                  SSIxClk      SSIxFss      SSIxRx       SSIxTx
static const u8 spi_gpio_clk_pin[] = { GPIO_PIN_2, GPIO_PIN_0 };
#define SPIS_COUNT 		1

static void spis_init()
{
  unsigned i;

  for( i = 0; i < SPIS_COUNT; i ++ )
  {
    SysCtlPeripheralEnable(spi_sysctl[ i ]);
  }
}

int platform_spi_exists( unsigned id )
{
  return id < SPIS_COUNT;
}

u32 platform_spi_setup( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits )
{
  unsigned protocol;
  
  if( cpol == 0 )
    protocol = cpha ? SSI_FRF_MOTO_MODE_1 : SSI_FRF_MOTO_MODE_0;
  else
    protocol = cpha ? SSI_FRF_MOTO_MODE_3 : SSI_FRF_MOTO_MODE_2;
  mode = mode == PLATFORM_SPI_MASTER ? SSI_MODE_MASTER : SSI_MODE_SLAVE;  
  SSIDisable( spi_base[ id ] );

  GPIOPinTypeSSI( spi_gpio_base[ id ], spi_gpio_pins[ id ] );
  
  // FIXME: not sure this is always "right"
  GPIOPadConfigSet(spi_gpio_base[ id ], spi_gpio_clk_pin[ id ], GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);    

  SSIConfigSetExpClk( spi_base[ id ], SysCtlClockGet(), protocol, mode, clock, databits );
  SSIEnable( spi_base[ id ] );
  return clock;
}

spi_data_type platform_spi_send_recv( unsigned id, spi_data_type data )
{
  SSIDataPut( spi_base[ id ], data );
  SSIDataGet( spi_base[ id ], &data );
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
// Different configurations for LM3S8962 (2 UARTs) and LM3S6965 (3 UARTs)

#if ELUA_CPU == LM3S8962
  #define UARTS_COUNT 2
#elif ELUA_CPU == LM3S6965
  #define UARTS_COUNT 3
#endif

// All possible LM3S uarts defs
static const u32 uart_base[] = { UART0_BASE, UART1_BASE, UART2_BASE };
static const u32 uart_sysctl[] = { SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2 };
static const u32 uart_gpio_base[] = { GPIO_PORTA_BASE, GPIO_PORTD_BASE, GPIO_PORTG_BASE };
static const u8 uart_gpio_pins[] = { GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_0 | GPIO_PIN_1 };

static void uarts_init()
{
  unsigned i;

  for( i = 0; i < UARTS_COUNT; i ++ )
  {
    SysCtlPeripheralEnable(uart_sysctl[ i ]);
  }

  // Special case for UART 0
  // Configure the UART for 115,200, 8-N-1 operation.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                     (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                      UART_CONFIG_PAR_NONE)); 
}

int platform_uart_exists( unsigned id )
{
  return id < UARTS_COUNT;
}

u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits )
{
  u32 config;
  
  GPIOPinTypeUART(uart_gpio_base [ id ], uart_gpio_pins[ id ]);

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
    
  return UARTConfigSetExpClk(uart_base[ id ], SysCtlClockGet(), baud, config);
}

void platform_uart_send( unsigned id, u8 data )
{
  UARTCharPut( uart_base[ id ], data );
}

int platform_uart_recv( unsigned id, unsigned timer_id, int timeout )
{
  u32 base = uart_base[ id ];
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
// Timers
// Same on LM3S8962 and LM3S6965

// All possible LM3S timers defs
static const u32 timer_base[] = { TIMER0_BASE, TIMER1_BASE, TIMER2_BASE, TIMER3_BASE };
static const u32 timer_sysctl[] = { SYSCTL_PERIPH_TIMER0, SYSCTL_PERIPH_TIMER1, SYSCTL_PERIPH_TIMER2, SYSCTL_PERIPH_TIMER3 };
#define TIMERS_COUNT			4

static void timers_init()
{
  unsigned i;

  for( i = 0; i < TIMERS_COUNT; i ++ )
  {
	SysCtlPeripheralEnable(timer_sysctl[ i ]);
    TimerConfigure(timer_base[ i ], TIMER_CFG_32_BIT_PER);
    TimerEnable(timer_base[ i ], TIMER_A);
  }
}

int platform_timer_exists( unsigned id )
{
  return id < TIMERS_COUNT;
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
// PWMs
// Same on LM3S8962 and LM3S6965

#define PLATFORM_NUM_PWMS               6

// SYSCTL div data and actual div factors
const static u32 pwm_div_ctl[] = { SYSCTL_PWMDIV_1, SYSCTL_PWMDIV_2, SYSCTL_PWMDIV_4, SYSCTL_PWMDIV_8, SYSCTL_PWMDIV_16, SYSCTL_PWMDIV_32, SYSCTL_PWMDIV_64 };
const static u8 pwm_div_data[] = { 1, 2, 4, 8, 16, 32, 64 };
// Port/pin information for all channels
const static u32 pwm_ports[] =  { GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTB_BASE, GPIO_PORTB_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE };
const static u8 pwm_pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1 };
// PWM generators
const static u16 pwm_gens[] = { PWM_GEN_0, PWM_GEN_1, PWM_GEN_2 };
// PWM outputs
const static u16 pwm_outs[] = { PWM_OUT_0, PWM_OUT_1, PWM_OUT_2, PWM_OUT_3, PWM_OUT_4, PWM_OUT_5 };

static void pwms_init()
{
  SysCtlPeripheralEnable( SYSCTL_PERIPH_PWM );
  SysCtlPWMClockSet( SYSCTL_PWMDIV_1 );
}

// Helper function: return the PWM clock
static u32 platform_pwm_get_clock()
{
  unsigned i;
  u32 clk;
  
  clk = SysCtlPWMClockGet();
  for( i = 0; i < sizeof( pwm_div_ctl ) / sizeof( u32 ); i ++ )
    if( clk == pwm_div_ctl[ i ] )
      break;
  return SysCtlClockGet() / pwm_div_data[ i ];
}

// Helper function: set the PWM clock
static u32 platform_pwm_set_clock( u32 clock )
{
  unsigned i, min_i;
  u32 sysclk;
  
  sysclk = SysCtlClockGet();
  for( i = min_i = 0; i < sizeof( pwm_div_data ) / sizeof( u8 ); i ++ )
    if( ABSDIFF( clock, sysclk / pwm_div_data[ i ] ) < ABSDIFF( clock, sysclk / pwm_div_data[ min_i ] ) )
      min_i = i;
  SysCtlPWMClockSet( pwm_div_ctl[ min_i ] );
  return sysclk / pwm_div_data[ min_i ];
}

int platform_pwm_exists( unsigned id )
{
  return id < PLATFORM_NUM_PWMS; 
}

u32 platform_pwm_setup( unsigned id, u32 frequency, unsigned duty )
{
  u32 pwmclk = platform_pwm_get_clock();
  u32 period;
  
  // Set pin as PWM
  GPIOPinTypePWM( pwm_ports[ id ], pwm_pins[ id ] );
  // Compute period
  period = pwmclk / frequency;
  // Set the period
  PWMGenConfigure( PWM_BASE, pwm_gens[ id >> 1 ], PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC );
  PWMGenPeriodSet( PWM_BASE, pwm_gens[ id >> 1 ], period );
  // Set duty cycle
  PWMPulseWidthSet( PWM_BASE, pwm_outs[ id ], ( period * duty ) / 100 );
  // Return actual frequency
  return pwmclk / period;
}

u32 platform_pwm_op( unsigned id, int op, u32 data )
{
  u32 res = 0;
  
  switch( op )
  {
    case PLATFORM_PWM_OP_SET_CLOCK:
      res = platform_pwm_set_clock( data );
      break;
            
    case PLATFORM_PWM_OP_GET_CLOCK:
      res = platform_pwm_get_clock();
      break;
      
    case PLATFORM_PWM_OP_START:
      PWMOutputState( PWM_BASE, 1 << id, true );
      PWMGenEnable( PWM_BASE, pwm_gens[ id >> 1 ] );
      break;
      
    case PLATFORM_PWM_OP_STOP:
      PWMOutputState( PWM_BASE, 1 << id, false );
      PWMGenDisable( PWM_BASE, pwm_gens[ id >> 1 ] );
      break;
  }
  
  return res;
}

// *****************************************************************************
// CPU specific functions

void platform_cpu_enable_interrupts()
{
  IntMasterEnable();
}

void platform_cpu_disable_interrupts()
{
  IntMasterDisable();
}

u32 platform_cpu_get_frequency()
{
  return SysCtlClockGet();
}

// ****************************************************************************
// Ethernet functions

static void eth_init()
{
#ifdef BUILD_UIP
  u32 user0, user1, temp;
  static struct uip_eth_addr sTempAddr;     
  
  // Enable and reset the controller
  SysCtlPeripheralEnable( SYSCTL_PERIPH_ETH );
  SysCtlPeripheralReset( SYSCTL_PERIPH_ETH );
  
  // Enable Ethernet LEDs
  GPIODirModeSet( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_HW );
  GPIOPadConfigSet( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );

  // Configure SysTick for a periodic interrupt.
  SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
  SysTickEnable();
  SysTickIntEnable();
  
  // Intialize the Ethernet Controller and disable all Ethernet Controller interrupt sources.
  EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                     ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
  temp = EthernetIntStatus(ETH_BASE, false);
  EthernetIntClear(ETH_BASE, temp);  
  
  // Initialize the Ethernet Controller for operation.
  EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

  // Configure the Ethernet Controller for normal operation.
  // - Full Duplex
  // - TX CRC Auto Generation
  // - TX Padding Enabled
  EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                               ETH_CFG_TX_PADEN));

  // Enable the Ethernet Controller.
  EthernetEnable(ETH_BASE);

  // Enable the Ethernet interrupt.
  IntEnable(INT_ETH);

  // Enable the Ethernet RX Packet interrupt source.
  EthernetIntEnable(ETH_BASE, ETH_INT_RX);  

  // Enable all processor interrupts.
  IntMasterEnable();

  // Configure the hardware MAC address for Ethernet Controller filtering of
  // incoming packets.
  //
  // For the Ethernet Eval Kits, the MAC address will be stored in the
  // non-volatile USER0 and USER1 registers.  These registers can be read
  // using the FlashUserGet function, as illustrated below.
  FlashUserGet(&user0, &user1);
  
  // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
  // address needed to program the hardware registers, then program the MAC
  // address into the Ethernet Controller registers.
  sTempAddr.addr[0] = ((user0 >>  0) & 0xff);
  sTempAddr.addr[1] = ((user0 >>  8) & 0xff);
  sTempAddr.addr[2] = ((user0 >> 16) & 0xff);
  sTempAddr.addr[3] = ((user1 >>  0) & 0xff);
  sTempAddr.addr[4] = ((user1 >>  8) & 0xff);
  sTempAddr.addr[5] = ((user1 >> 16) & 0xff);  

  // Program the hardware with it's MAC address (for filtering).
  EthernetMACAddrSet(ETH_BASE, (unsigned char *)&sTempAddr);  

  // Initialize the eLua uIP layer
  elua_uip_init( &sTempAddr );
#endif
}

#ifdef BUILD_UIP
static int eth_timer_fired;

void platform_eth_send_packet( const void* src, u32 size )
{
  EthernetPacketPut( ETH_BASE, uip_buf, uip_len );    
}

u32 platform_eth_get_packet_nb( void* buf, u32 maxlen )
{
  return EthernetPacketGetNonBlocking( ETH_BASE, uip_buf, sizeof( uip_buf ) );
}

void platform_eth_force_interrupt()
{
  HWREG( NVIC_SW_TRIG) |= INT_ETH - 16;  
}

u32 platform_eth_get_elapsed_time()
{
  if( eth_timer_fired )
  {
    eth_timer_fired = 0;
    return SYSTICKMS;
  }
  else
    return 0;
}

void SysTickIntHandler()
{
  // Indicate that a SysTick interrupt has occurred.
  eth_timer_fired = 1;

  // Generate a fake Ethernet interrupt.  This will perform the actual work
  // of incrementing the timers and taking the appropriate actions.
  platform_eth_force_interrupt();
}

void EthernetIntHandler()
{
  u32 temp;
  
  // Read and Clear the interrupt.
  temp = EthernetIntStatus( ETH_BASE, false );
  EthernetIntClear( ETH_BASE, temp );

  // Call the UIP main loop  
  elua_uip_mainloop();
}

#else  // #ifdef ELUA_UIP

void SysTickIntHandler()
{
}

void EthernetIntHandler()
{
}
#endif // #ifdef ELUA_UIP

// ****************************************************************************
// Allocator support

extern char end[];

void* platform_get_first_free_ram( unsigned id )
{
  return id > 0 ?  NULL : ( void* )end;
}

#define STACK_SIZE 256
#define SRAM_SIZE ( 64 * 1024 )
void* platform_get_last_free_ram( unsigned id )
{
  return id > 0 ? NULL : ( void* )( SRAM_BASE + SRAM_SIZE - STACK_SIZE - 1 );
}
