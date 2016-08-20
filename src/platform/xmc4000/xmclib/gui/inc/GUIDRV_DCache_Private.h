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
File        : GUIDRV_DCache_Private.h
Purpose     : Private declarations for GUIDRV_DCache driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_DCACHE_PRIVATE_H
#define GUIDRV_DCACHE_PRIVATE_H

#include "GUIDRV_DCache.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
/*********************************************************************
*
*       DRIVER_CONTEXT
*/
typedef struct {
  //
  // Data
  //
  int xSize, ySize;       // Display size
  int vxSize, vySize;     // Virtual display size
  int BitsPerPixelDriver;
  int BitsPerPixel;
  int NumColors;
  LCD_PIXELINDEX IndexMask;
  U32 MemSize;
  GUI_RECT rDirty;
  const GUI_DEVICE_API * pMemdev_API;
  //
  // Cache mamagement
  //
  void     (* pfFlush        )(GUI_DEVICE * pDevice);
  void     (* pfSendCacheRect)(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1);
  //
  // Setting the rectangle to be filled up within the real driver
  //
  void     (* pfSetRect      )(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1, int OnOff);
  //
  // Mode dependent drawing functions
  //
  void     (* pfDrawBitmap   )(GUI_DEVICE * pDevice, int x0, int y0, int xsize, int ysize, int _BitsPerPixel, int BytesPerLine, const U8 * pData, int Diff, const LCD_PIXELINDEX * pTrans);
  void     (* pfFillRect     )(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1);
  unsigned (* pfGetPixelIndex)(GUI_DEVICE * pDevice, int x, int y);
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);
  //
  // Request information
  //
  I32      (* pfGetDevProp   )(GUI_DEVICE * pDevice, int Index);
  //
  // Initialization
  //
  void     (* pfInit)         (GUI_DEVICE * pDevice);
  //
  // Conversion array from cache to real display driver
  //
  LCD_PIXELINDEX * pConvert;
  LCD_PIXELINDEX * pIndex;
  //
  // Cache
  //
  U8 * pVRAM;
  U8 * pVRAM_Lock;
  int CacheLocked;
  int CacheStat;
  int CacheDirty;
  //
  // The driver which is used for the actual drawing operations
  //
  GUI_DEVICE * pDriver;
} DRIVER_CONTEXT;

/*********************************************************************
*
*       Private interface
*
**********************************************************************
*/
void GUIDRV_DCache__AddDirtyRect  (DRIVER_CONTEXT * pContext, int x0, int y0, int x1, int y1);
void GUIDRV_DCache__ClearDirtyRect(DRIVER_CONTEXT * pContext);
void GUIDRV_DCache__InitOnce      (GUI_DEVICE * pDevice);

#if defined(__cplusplus)
}
#endif

#endif /* GUIDRV_DCACHE_PRIVATE_H */

/*************************** End of file ****************************/

