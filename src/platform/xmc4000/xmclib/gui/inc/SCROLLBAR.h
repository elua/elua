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
File        : SCROLLBAR.h
Purpose     : SCROLLBAR include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SCROLLBAR_CI_THUMB 0
#define SCROLLBAR_CI_SHAFT 1
#define SCROLLBAR_CI_ARROW 2

/*********************************************************************
*
*       States
*/
#define SCROLLBAR_STATE_PRESSED   WIDGET_STATE_USER0

/*********************************************************************
*
*       Create / Status flags
*/
#define SCROLLBAR_CF_VERTICAL     WIDGET_CF_VERTICAL
#define SCROLLBAR_CF_FOCUSSABLE   WIDGET_STATE_FOCUSSABLE

/************************************************************
*
*       Skinning property indices
*/
#define SCROLLBAR_SKINFLEX_PI_PRESSED   0
#define SCROLLBAR_SKINFLEX_PI_UNPRESSED 1

/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_HMEM SCROLLBAR_Handle;

typedef struct {
  GUI_COLOR aColorFrame[3];
  GUI_COLOR aColorUpper[2];
  GUI_COLOR aColorLower[2];
  GUI_COLOR aColorShaft[2];
  GUI_COLOR ColorArrow;
  GUI_COLOR ColorGrasp;
} SCROLLBAR_SKINFLEX_PROPS;

typedef struct {
  int IsVertical;
  int State;
} SCROLLBAR_SKINFLEX_INFO;

/*********************************************************************
*
*       Create functions
*
**********************************************************************
*/
SCROLLBAR_Handle SCROLLBAR_Create        (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int Id, int WinFlags, int SpecialFlags);
SCROLLBAR_Handle SCROLLBAR_CreateAttached(WM_HWIN hParent, int SpecialFlags);
SCROLLBAR_Handle SCROLLBAR_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id);
SCROLLBAR_Handle SCROLLBAR_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int NumExtraBytes);
SCROLLBAR_Handle SCROLLBAR_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void SCROLLBAR_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/

/* Methods changing properties */

void      SCROLLBAR_AddValue   (SCROLLBAR_Handle hObj, int Add);
void      SCROLLBAR_Dec        (SCROLLBAR_Handle hObj);
void      SCROLLBAR_Inc        (SCROLLBAR_Handle hObj);
int       SCROLLBAR_GetUserData(SCROLLBAR_Handle hObj, void * pDest, int NumBytes);
GUI_COLOR SCROLLBAR_SetColor   (SCROLLBAR_Handle hObj, int Index, GUI_COLOR Color);
void      SCROLLBAR_SetNumItems(SCROLLBAR_Handle hObj, int NumItems);
void      SCROLLBAR_SetPageSize(SCROLLBAR_Handle hObj, int PageSize);
void      SCROLLBAR_SetValue   (SCROLLBAR_Handle hObj, int v);
int       SCROLLBAR_SetWidth   (SCROLLBAR_Handle hObj, int Width);
void      SCROLLBAR_SetState   (SCROLLBAR_Handle hObj, const WM_SCROLL_STATE* pState);
int       SCROLLBAR_SetUserData(SCROLLBAR_Handle hObj, const void * pSrc, int NumBytes);

/*********************************************************************
*
*       Member functions: Skinning
*
**********************************************************************
*/
void SCROLLBAR_GetSkinFlexProps     (SCROLLBAR_SKINFLEX_PROPS * pProps, int Index);
void SCROLLBAR_SetSkinClassic       (SCROLLBAR_Handle hObj);
void SCROLLBAR_SetSkin              (SCROLLBAR_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);
int  SCROLLBAR_DrawSkinFlex         (const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);
void SCROLLBAR_SetSkinFlexProps     (const SCROLLBAR_SKINFLEX_PROPS * pProps, int Index);
void SCROLLBAR_SetDefaultSkinClassic(void);
WIDGET_DRAW_ITEM_FUNC * SCROLLBAR_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);

#define SCROLLBAR_SKIN_FLEX    SCROLLBAR_DrawSkinFlex

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
int       SCROLLBAR_GetDefaultWidth(void);
int       SCROLLBAR_GetThumbSizeMin(void);
GUI_COLOR SCROLLBAR_SetDefaultColor(GUI_COLOR Color, unsigned int Index); /* Not yet documented */
int       SCROLLBAR_SetDefaultWidth(int DefaultWidth);
int       SCROLLBAR_SetThumbSizeMin(int ThumbSizeMin);

/*********************************************************************
*
*       Query state
*
**********************************************************************
*/
int SCROLLBAR_GetNumItems(SCROLLBAR_Handle hObj);
int SCROLLBAR_GetPageSize(SCROLLBAR_Handle hObj);
int SCROLLBAR_GetValue   (SCROLLBAR_Handle hObj);

/*********************************************************************
*
*       Macros for compatibility
*
**********************************************************************
*/
#define SCROLLBAR_BKCOLOR0_DEFAULT SCROLLBAR_COLOR_ARROW_DEFAULT
#define SCROLLBAR_BKCOLOR1_DEFAULT SCROLLBAR_COLOR_SHAFT_DEFAULT
#define SCROLLBAR_COLOR0_DEFAULT   SCROLLBAR_COLOR_THUMB_DEFAULT

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // SCROLLBAR_H

/*************************** End of file ****************************/
