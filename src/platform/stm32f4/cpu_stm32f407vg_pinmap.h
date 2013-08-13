#ifndef __CPU_STM32F407VG_PINMAP_H__
#define __CPU_STM32F407VG_PINMAP_H__

#include "pinmap.h"

#ifdef IMPLEMENT_PINMAPS

static const pin_function pinfuncs_PA_0[] = {
  { _UART1_CTS, 7 },
  { _UART3_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_1[] = {
  { _UART1_RTS, 7 },
  { _UART3_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_2[] = {
  { _UART1_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_3[] = {
  { _UART1_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_4[] = {
  { _SPI0_SS, 5 },
  { _SPI2_SS, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_5[] = {
  { _SPI0_SCK, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_6[] = {
  { _SPI0_MISO, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_7[] = {
  { _SPI0_MOSI, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_9[] = {
  { _UART0_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_10[] = {
  { _UART0_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_11[] = {
  { _UART0_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_12[] = {
  { _UART0_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PA_15[] = {
  { _SPI0_SS, 5 },
  { _SPI2_SS, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_3[] = {
  { _SPI0_SCK, 5 },
  { _SPI2_SCK, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_4[] = {
  { _SPI0_MISO, 5 },
  { _SPI2_MISO, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_5[] = {
  { _SPI0_MOSI, 5 },
  { _SPI2_MOSI, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_6[] = {
  { _UART0_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_7[] = {
  { _UART0_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_9[] = {
  { _SPI1_SS, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_10[] = {
  { _UART2_TX, 7 },
  { _SPI1_SCK, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_11[] = {
  { _UART2_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_12[] = {
  { _SPI1_SS, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_13[] = {
  { _UART2_CTS, 7 },
  { _SPI1_SCK, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_14[] = {
  { _UART2_RTS, 7 },
  { _SPI1_MISO, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PB_15[] = {
  { _SPI1_MOSI, 5 },
  { _SPI1_MISO, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_2[] = {
  { _SPI1_MISO, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_3[] = {
  { _SPI1_MOSI, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_6[] = {
  { _UART5_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_7[] = {
  { _UART5_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_10[] = {
  { _UART2_TX, 7 },
  { _UART3_TX, 8 },
  { _SPI2_SCK, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_11[] = {
  { _UART2_RX, 7 },
  { _UART3_RX, 8 },
  { _SPI2_MISO, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PC_12[] = {
  { _UART4_TX, 8 },
  { _SPI2_MOSI, 6 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_2[] = {
  { _UART4_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_3[] = {
  { _UART1_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_4[] = {
  { _UART1_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_5[] = {
  { _UART1_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_6[] = {
  { _UART1_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_8[] = {
  { _UART2_TX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_9[] = {
  { _UART2_RX, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_11[] = {
  { _UART2_CTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PD_12[] = {
  { _UART2_RTS, 7 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_8[] = {
  { _UART5_RTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_9[] = {
  { _UART5_RX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_12[] = {
  { _UART5_RTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_13[] = {
  { _UART5_CTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_14[] = {
  { _UART5_TX, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PG_15[] = {
  { _UART5_CTS, 8 },
  _NOFUNC
};

static const pin_function pinfuncs_PI_0[] = {
  { _SPI1_SS, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PI_1[] = {
  { _SPI1_SCK, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PI_2[] = {
  { _SPI1_MISO, 5 },
  _NOFUNC
};

static const pin_function pinfuncs_PI_3[] = {
  { _SPI1_MOSI, 5 },
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
  PINDATA( PA_4 ),
  PINDATA( PA_5 ),
  PINDATA( PA_6 ),
  PINDATA( PA_7 ),
  PINDATA( PA_9 ),
  PINDATA( PA_10 ),
  PINDATA( PA_11 ),
  PINDATA( PA_12 ),
  PINDATA( PA_15 ),
  PINDATA( PB_3 ),
  PINDATA( PB_4 ),
  PINDATA( PB_5 ),
  PINDATA( PB_6 ),
  PINDATA( PB_7 ),
  PINDATA( PB_9 ),
  PINDATA( PB_10 ),
  PINDATA( PB_11 ),
  PINDATA( PB_12 ),
  PINDATA( PB_13 ),
  PINDATA( PB_14 ),
  PINDATA( PB_15 ),
  PINDATA( PC_2 ),
  PINDATA( PC_3 ),
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
  PINDATA( PI_0 ),
  PINDATA( PI_1 ),
  PINDATA( PI_2 ),
  PINDATA( PI_3 )
};

#endif // #ifdef IMPLEMENT_PINMAPS

#endif // #ifndef __CPU_STM32F407VG_PINMAP_H__

