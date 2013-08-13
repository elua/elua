// Pin mapping functions

#ifndef __PINMAP_H__
#define __PINMAP_H__

#include "platform.h"
#include "type.h"
#include "pin_names.h"

// Various peripheral definitions
enum
{
  PINMAP_NONE,
  PINMAP_GPIO,
  PINMAP_UART,
  PINMAP_TMR,
  PINMAP_CAN,
  PINMAP_PWM,
  PINMAP_SPI,
  PINMAP_I2C
};
#define PINMAP_PERIPHERAL_NAMES     { "NONE", "GPIO", "UART", "TMR", "CAN", "PWM", "SPI", "I2C" }

// UART pin IDs
enum
{
  PINMAP_UART_RX,
  PINMAP_UART_TX,
  PINMAP_UART_RTS,
  PINMAP_UART_CTS,
  PINMAP_UART_TOTAL
};
#define PINMAP_UART_PIN_NAMES       { "RX", "TX", "RTS", "CTS" }

// SPI pin IDs
enum
{
  PINMAP_SPI_MOSI,
  PINMAP_SPI_MISO,
  PINMAP_SPI_SCK,
  PINMAP_SPI_SS,
  PINMAP_SPI_TOTAL
};
#define PINMAP_SPI_PIN_NAMES        { "MOSI", "MISO", "SCK", "SS" }

// Internal data structures
typedef struct 
{
  u8 peripheral;
  u8 id;
  u8 pin_id;
  pin_be_info pin_data;
} pin_function;

typedef struct
{
  u16 pin;
  const pin_function* pfuncs;
} pin_info;

#define _NOFUNC               { PINMAP_NONE, 0, 0, 0 }
#define _NOPIN                { NO_PIN, NULL }
#define PINDATA( name )       { name, pinfuncs_##name }

#define PINMAP_IGNORE_PIN     ( -1 )

// Public interface
int pinmap_initial_config();
int pinmap_uart( int id, int rx, int tx, int rts, int cts );
int pinmap_spi( int id, int mosi, int miso, int sck );
int pinmap_get_num_pins();
const pin_info* pinmap_get_at( int i );

#endif

