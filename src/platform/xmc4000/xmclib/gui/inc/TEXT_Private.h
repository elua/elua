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
File        : TEXT.h
Purpose     : TEXT include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef TEXT_PRIVATE_H
#define TEXT_PRIVATE_H

#include "TEXT.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  const GUI_FONT * pFont;
  GUI_COLOR TextColor;
  GUI_COLOR BkColor;
  GUI_WRAPMODE WrapMode;
} TEXT_PROPS;

typedef struct {
  WIDGET Widget;
  TEXT_PROPS Props;
  WM_HMEM hpText;
  I16 Align;
} TEXT_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define TEXT_INIT_ID(p) p->Widget.DebugId = TEXT_ID
#else
  #define TEXT_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  TEXT_OBJ * TEXT_LockH(TEXT_Handle h);
  #define TEXT_LOCK_H(h)   TEXT_LockH(h)
#else
  #define TEXT_LOCK_H(h)   (TEXT_OBJ *)GUI_LOCK_H(h)
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern TEXT_PROPS TEXT__DefaultProps;

#endif   /* if GUI_WINSUPPORT */
#endif   /* TEXT_PRIVATE_H */
