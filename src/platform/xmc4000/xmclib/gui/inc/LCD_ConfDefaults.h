/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.30 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software  belongs to SEGGER.
emWin is protected by international copyright laws.

This  file  has been  licensed to  Infineon Technologies AG,  a german
company at the address Am Campeon 1-12 in 85579 Neubiberg, Germany and
is  sublicensed  and  distributed  by   Infineon  Technologies  AG  in
accordance with the DAVE (TM) 3 Software License Agreement to be used 
for and with Infineon's Cortex-M0, M0+ and M4 based 32-bit microcon-
troller products only.

Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information

Licensor:                 SEGGER Microcontroller GmbH & Co. KG
Licensed to:              Infineon Technologies AG, Am Campeon 1-12, 85579 Neubiberg
Licensed SEGGER software: emWin
License number:           GUI-00324
License model:            Buyout SRC [Buyout Source Code License]
Licensed product:         -
Licensed platform:        Infineon's Cortex M0, M0+, M4 based  32-bit  microcontroller  products
Licensed number of seats: -
----------------------------------------------------------------------
File        : LCD_ConfDefaults.h
Purpose     : Valid LCD configuration and defaults
----------------------------------------------------------------------
*/

#ifndef LCD_CONFIG_DEFAULTS_H
#define LCD_CONFIG_DEFAULTS_H

#include "LCDConf.h"            /* Configuration header file */

/**********************************************************
*
*       Configuration defaults
*/
#ifndef   LCD_MIRROR_X
  #define LCD_MIRROR_X 0
#endif
#ifndef   LCD_MIRROR_Y
  #define LCD_MIRROR_Y 0
#endif
#ifndef   LCD_SWAP_XY
  #define LCD_SWAP_XY 0
#endif
#ifndef   LCD_FIRSTCOM0
  #define LCD_FIRSTCOM0 0
#endif
#ifndef   LCD_FIRSTSEG0
  #define LCD_FIRSTSEG0 0
#endif
#ifndef   LCD_SWAP_RB
  #define LCD_SWAP_RB 0
#endif
#ifndef   LCD_DISPLAY_INDEX
  #define LCD_DISPLAY_INDEX 0
#endif
#ifndef   LCD_ENDIAN_BIG
  #define LCD_ENDIAN_BIG 0
#endif
#ifndef   LCD_ALLOW_NON_OPTIMIZED_MODE
  #define LCD_ALLOW_NON_OPTIMIZED_MODE 1
#endif

#endif /* LCD_CONFIG_DEFAULTS_H */

/*************************** End of file ****************************/
