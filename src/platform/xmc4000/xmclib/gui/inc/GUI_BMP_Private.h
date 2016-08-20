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
File        : GUI_BMP_Private.h
Purpose     : Private header file for GUI_BMP... functions
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUI_BMP_PRIVATE_H
#define GUI_BMP_PRIVATE_H

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define BI_RGB       0
#define BI_RLE8      1
#define BI_RLE4      2
#define BI_BITFIELDS 3

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
//
// Default parameter structure for reading data from memory
//
typedef struct {
  const U8 * pFileData;
} GUI_BMP_PARAM;

//
// Context structure for getting stdio input
//
typedef struct {
  GUI_GET_DATA_FUNC * pfGetData; // Function pointer
  U32                 Off;       // Data pointer
  void              * pParam;    // Parameter pointer passed to function
} GUI_BMP_CONTEXT;

//
// Parameter structure for passing several required variables to the
// functions _DrawLine_RGB() and _DrawLine_ARGB() (in GUI_BMP_EnableAlpha.c).
//
typedef struct {
  const U8            * pSrc;           // Pointer to data
  I32                   xSrc;           // Used to read data
  int                   ySrc;           // Used to read data
  I32                   xSize;
  U32                   BytesPerPixel;
  tLCDDEV_Color2Index * pfColor2Index;
  tLCDDEV_Index2Color * pfIndex2Color;  // Used to manage bitfield conversion
  LCD_API_NEXT_PIXEL  * pNextPixel_API;
  int                   x0;             // Used to draw data
  int                   y0;             // Used to draw data
  int                   x1;             // Used to draw data
  int                   y1;             // Used to draw data
} GUI_DRAWLINE_INFO;

/*********************************************************************
*
*       Interface
*
**********************************************************************
*/
int GUI_BMP__GetData    (void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off);
int GUI_BMP__Init       (GUI_BMP_CONTEXT * pContext, I32 * pWidth, I32 * pHeight, U16 * pBitCount, int * pNumColors, int * pCompression);
int GUI_BMP__ReadData   (GUI_BMP_CONTEXT * pContext, int NumBytes, const U8 ** ppData, unsigned StartOfFile);
int GUI_BMP__ReadPalette(GUI_BMP_CONTEXT * pContext, int NumColors);

#endif /* GUI_BMP_PRIVATE_H */

/*************************** End of file ****************************/
