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
File        : WM_GUI.h
Purpose     : Windows manager include for low level GUI routines
----------------------------------------------------------------------
*/

#ifndef WM_GUI_H            /* Make sure we only include it once */
#define WM_GUI_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

int       WM__InitIVRSearch(const GUI_RECT* pMaxRect);
int       WM__GetNextIVR   (void);
int       WM__GetOrgX_AA(void);
int       WM__GetOrgY_AA(void);

#define WM_ITERATE_START(pRect)                   \
  {                                               \
    if (WM__InitIVRSearch(pRect))                 \
      do {

#define WM_ITERATE_END()                          \
    } while (WM__GetNextIVR());                   \
  }

#define WM_ADDORGX(x)       (x += GUI_pContext->xOff)
#define WM_ADDORGY(y)       (y += GUI_pContext->yOff)
#define WM_ADDORG(x0,y0)    WM_ADDORGX(x0); WM_ADDORGY(y0)
#define WM_ADDORGX_AA(x)    (x += WM__GetOrgX_AA())
#define WM_ADDORGY_AA(y)    (y += WM__GetOrgY_AA())
#define WM_ADDORG_AA(x0,y0) WM_ADDORGX_AA(x0); WM_ADDORGY_AA(y0)
#define WM_SUBORGX(x)       (x -= GUI_pContext->xOff)
#define WM_SUBORGY(y)       (y -= GUI_pContext->yOff)
#define WM_SUBORG(x0,y0)    WM_SUBORGX(x0); WM_SUBORGY(y0)

#if defined(__cplusplus)
  }
#endif


#endif   /* Avoid multiple inclusion */

/*************************** End of file ****************************/
