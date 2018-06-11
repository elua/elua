// Validate eLua configuration options
// Should be included in main.c, as this is the first file that's compiled, so
// any configuration errors are caught right at the beginning of the build
// process

#ifndef __VALIDATE_H__
#define __VALIDATE_H__

#include "platform_conf.h"

// The maximum 32 GPIO ports limitation is given by the port_mask variable in src/modules/pio.c
#if NUM_PIO > 32
#error "Can't have more than 32 GPIO ports"
#endif

// Can't define more than one console devices
#if defined( BUILD_CON_TCP ) && defined( BUILD_CON_GENERIC )
#error "Can't have two console devices (don't enable BUILD_CON_TCP and BUILD_CON_GENERIC in board file at the same time)"
#endif // #if defined( BUILD_CON_TCP ) && defined( BUILD_CON_GENERIC )

// For TCP console we need to enable TCP support
#ifdef BUILD_CON_TCP
  #ifndef BUILD_UIP
  #error "BUILD_CON_TCP requires BUILD_UIP to be defined in cpu, board headers" 
  #endif // #ifndef BUILD_UIP
#endif // #ifdef BUILD_CON_TCP

// If TCP console is enabled, we don't need xmodem or term
// (they can still function separately over UART, but this doesn't make sense)
#ifdef BUILD_CON_TCP
  #ifdef BUILD_XMODEM
  #error "XMODEM doesn't work with TCP console. Disable BUILD_XMODEM in cpu, board headers"
  #endif // #ifdef BUILD_XMODME
  #ifdef BUILD_TERM
  #error "ANSI terminal support doesn't work (yet) with TCP console. Disable BUILD_TERM in platform_conf.h"
  #endif // #ifdef BUILD_TERM
#endif // #ifdef BUILD_CON_TCP

// If console is on USB serial port, check that USB serial port is also enabled
#if CON_UART_ID == CDC_UART_ID && !defined(BUILD_USB_CDC)
#error The console is directed to USB, but the USB CDC module is not enabled.
#error Add cdc=true to the board file.
#endif

// For DHCP we need to have TCP/IP support
#ifdef BUILD_DHCPC
  #ifndef BUILD_UIP
  #error "DHCP client requires TCP/IP support (enable BUILD_UIP in cpu, board headers)"
  #endif // #ifndef BUILD_UIP
#endif // #ifdef BUILD_DHCPC

// For DNS we need to have TCP/IP support
#ifdef BUILD_DNS
  #ifndef BUILD_UIP
  #error "DNS resolver requires TCP/IP support (enable BUILD_UIP in cpu, board headers)"
  #endif // #ifndef BUILD_UIP
#endif // #ifdef BUILD_DNS

// For linenoise we need term
#ifdef BUILD_LINENOISE
  #ifndef BUILD_TERM
  #error "linenoise needs term support, define BUILD_TERM"
  #endif // #ifndef BUILD_TERM
#endif // #ifdef BUILD_LINENOISE

// For BUF_ENABLE_UART we also need C interrupt handlers support and specific INT_UART_RX support
#if defined( BUF_ENABLE_UART ) 
  #if !defined( BUILD_C_INT_HANDLERS )
  #error "Buffering support on UART neeeds C interrupt handlers support, define BUILD_C_INT_HANDLERS in your cpu, board headers"
  #endif
  #if !defined( INT_UART_RX )
  #error "Buffering support on UART needs support for the INT_UART_RX interrupt"
  #endif
#endif

// Virtual UARTs need buffering and a few specific macros
#if defined( BUILD_SERMUX )
  #if !defined( BUF_ENABLE_UART )
  #error "Virtual UARTs need buffering support, enable BUF_ENABLE_UART"  
  #endif
#endif

#if defined( BUILD_MMCFS ) && !defined( PLATFORM_HAS_SYSTIMER )
  #error "BUILD_MMCFS needs system timer support. Ensure your platform has this implemented and PLATFORM_HAS_SYSTIMER is defined"
#endif

// CON_BUF_SIZE needs BUF_ENABLE_UART and CON_UART_ID
#if defined( CON_BUF_SIZE )
  #if !defined( BUF_ENABLE_UART )
  #error "Console buffering needs BUF_ENABLE_UART"
  #endif
  #if !defined( CON_UART_ID )
  #error "Console buffering needs CON_UART_ID defined to the UART ID of the console device"
  #endif
#endif

// BUILD_ADVANCED_SHELL needs BUILD_SHELL
#if defined( BUILD_ADVANCED_SHELL ) && !defined( BUILD_SHELL )
  #error "BUILD_ADVANCED_SHELL needs BUILD_SHELL"
#endif

// The memory error callback can only be enabled when using either the muliple allocator or the simple allocator
// (but not the built-in allocator)
#if defined( MEM_ERROR_CALLBACK ) && !defined( USE_MULTIPLE_ALLOCATOR ) && !defined( USE_SIMPLE_ALLOCATOR )
  #error "A memory error callback can only be specified when using the multiple allocator or the simple allocator, but not the built-in allocator"
#endif
  
#endif // #ifndef __VALIDATE_H__

