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
File        : KNOB.h
Purpose     : KNOB include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef KNOB_PRIVATE_H
#define KNOB_PRIVATE_H

#include "KNOB.h"
#include "GUI_Private.h"

#if (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  I32 Snap;          // Position where the knob snaps
  I32 Period;        // Time it takes to stop the knob in ms
  GUI_COLOR BkColor; // The Bk color
  I32 Offset;        // the offset
  I32 MinRange;
  I32 MaxRange;
  I32 TickSize;      // Minimum movement range in 1/10 of degree
  I32 KeyValue;      // Range of movement for one key push
} KNOB_PROPS;

typedef struct {
  WIDGET Widget;
  KNOB_PROPS Props;
  WM_HMEM hContext;
  I32 Angle;
  I32 Value;
  int xSize;
  int ySize;
  GUI_MEMDEV_Handle hMemSrc;
  GUI_MEMDEV_Handle hMemDst;
  GUI_MEMDEV_Handle hMemBk;
} KNOB_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define KNOB_INIT_ID(p) p->Widget.DebugId = KNOB_ID
#else
  #define KNOB_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  KNOB_OBJ * KNOB_LockH(KNOB_Handle h);
  #define KNOB_LOCK_H(h)   KNOB_LockH(h)
#else
  #define KNOB_LOCK_H(h)   (KNOB_OBJ *)GUI_LOCK_H(h)
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern KNOB_PROPS KNOB__DefaultProps;

#endif   // (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)
#endif   // KNOB_PRIVATE_H
