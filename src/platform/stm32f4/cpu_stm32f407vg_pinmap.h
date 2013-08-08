#ifndef __CPU_STM32F407VG_PINMAP_H__
#define __CPU_STM32F407VG_PINMAP_H__

#include "pinmap.h"

#ifdef IMPLEMENT_PINMAPS

static const pin_function pinfuncs_PA_0[] = {
  { PINMAP_UART, 1, PINMAP_UART_CTS, 7 },
  { PINMAP_UART, 3, PINMAP_UART_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_1[] = {
  { PINMAP_UART, 1, PINMAP_UART_RTS, 7 },
  { PINMAP_UART, 3, PINMAP_UART_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_2[] = {
  { PINMAP_UART, 1, PINMAP_UART_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_3[] = {
  { PINMAP_UART, 1, PINMAP_UART_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_9[] = {
  { PINMAP_UART, 0, PINMAP_UART_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_10[] = {
  { PINMAP_UART, 0, PINMAP_UART_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_11[] = {
  { PINMAP_UART, 0, PINMAP_UART_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_12[] = {
  { PINMAP_UART, 0, PINMAP_UART_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_6[] = {
  { PINMAP_UART, 0, PINMAP_UART_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_7[] = {
  { PINMAP_UART, 0, PINMAP_UART_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_10[] = {
  { PINMAP_UART, 2, PINMAP_UART_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_11[] = {
  { PINMAP_UART, 2, PINMAP_UART_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_13[] = {
  { PINMAP_UART, 2, PINMAP_UART_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_14[] = {
  { PINMAP_UART, 2, PINMAP_UART_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_6[] = {
  { PINMAP_UART, 5, PINMAP_UART_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_7[] = {
  { PINMAP_UART, 5, PINMAP_UART_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_10[] = {
  { PINMAP_UART, 2, PINMAP_UART_TX, 7 },
  { PINMAP_UART, 3, PINMAP_UART_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_11[] = {
  { PINMAP_UART, 2, PINMAP_UART_RX, 7 },
  { PINMAP_UART, 3, PINMAP_UART_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_12[] = {
  { PINMAP_UART, 4, PINMAP_UART_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_2[] = {
  { PINMAP_UART, 4, PINMAP_UART_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_3[] = {
  { PINMAP_UART, 1, PINMAP_UART_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_4[] = {
  { PINMAP_UART, 1, PINMAP_UART_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_5[] = {
  { PINMAP_UART, 1, PINMAP_UART_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_6[] = {
  { PINMAP_UART, 1, PINMAP_UART_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_8[] = {
  { PINMAP_UART, 2, PINMAP_UART_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_9[] = {
  { PINMAP_UART, 2, PINMAP_UART_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_11[] = {
  { PINMAP_UART, 2, PINMAP_UART_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_12[] = {
  { PINMAP_UART, 2, PINMAP_UART_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_8[] = {
  { PINMAP_UART, 5, PINMAP_UART_RTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_9[] = {
  { PINMAP_UART, 5, PINMAP_UART_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_12[] = {
  { PINMAP_UART, 5, PINMAP_UART_RTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_13[] = {
  { PINMAP_UART, 5, PINMAP_UART_CTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_14[] = {
  { PINMAP_UART, 5, PINMAP_UART_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_15[] = {
  { PINMAP_UART, 5, PINMAP_UART_CTS, 8 },
  _NOFUNC
};

/*static const pin_function pinfuncs_[] = {
  { },
  _NOFUNC
};*/

static const pin_info pinfuncs[] =
{
  PINDATA( PA_0 ),
  PINDATA( PA_1 ),
  PINDATA( PA_2 ),
  PINDATA( PA_3 ),
  PINDATA( PA_9 ),
  PINDATA( PA_10 ),
  PINDATA( PA_11 ),
  PINDATA( PA_12 ),
  PINDATA( PB_6 ),
  PINDATA( PB_7 ),
  PINDATA( PB_10 ),
  PINDATA( PB_11 ),
  PINDATA( PB_13 ),
  PINDATA( PB_14 ),
  PINDATA( PC_6 ),
  PINDATA( PC_7 ),
  PINDATA( PC_10 ),
  PINDATA( PC_11 ),
  PINDATA( PC_12 ),
  PINDATA( PD_2 ),
  PINDATA( PD_3 ),
  PINDATA( PD_4 ),
  PINDATA( PD_5 ),
  PINDATA( PD_6 ),
  PINDATA( PD_8 ),
  PINDATA( PD_9 ),
  PINDATA( PD_11 ),
  PINDATA( PD_12 ),
  PINDATA( PG_8 ),
  PINDATA( PG_9 ),
  PINDATA( PG_12 ),
  PINDATA( PG_13 ),
  PINDATA( PG_14 ),
  PINDATA( PG_15 ),
  _NOPIN
};

#endif // #ifdef IMPLEMENT_PINMAPS

#endif // #ifndef __CPU_STM32F407VG_PINMAP_H__

