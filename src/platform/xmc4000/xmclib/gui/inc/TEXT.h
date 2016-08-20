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

#ifndef TEXT_H
#define TEXT_H

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"
#include "GUI_Debug.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/************************************************************
*
*       #defines
*
*************************************************************
*/

/************************************************************
*
*       Create / Status flags
*/
#define TEXT_CF_LEFT    GUI_TA_LEFT
#define TEXT_CF_RIGHT   GUI_TA_RIGHT
#define TEXT_CF_HCENTER GUI_TA_HCENTER

#define TEXT_CF_VCENTER GUI_TA_VCENTER
#define TEXT_CF_TOP     GUI_TA_TOP
#define TEXT_CF_BOTTOM  GUI_TA_BOTTOM


/*********************************************************************
*
*       Public Types
*
**********************************************************************

*/
typedef WM_HMEM TEXT_Handle;

/*********************************************************************
*
*       Standard member functions
*
**********************************************************************
*/

#define TEXT_EnableMemdev(hObj)  WM_EnableMemdev(hObj)
#define TEXT_DisableMemdev(hObj) WM_DisableMemdev(hObj)
#define TEXT_Delete(hObj)        WM_DeleteWindow(hObj)
#define TEXT_Paint(hObj)         WM_Paint(hObj)
#define TEXT_Invalidate(hObj)    WM_InvalidateWindow(hObj)

/*********************************************************************
*
*       Create functions
*
**********************************************************************
*/
TEXT_Handle TEXT_Create        (int x0, int y0, int xSize, int ySize, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateAsChild (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, const char * pText);
TEXT_Handle TEXT_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, const char * pText, int NumExtraBytes);
TEXT_Handle TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void TEXT_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/

/* Methods changing properties */

GUI_COLOR        TEXT_GetBkColor  (TEXT_Handle hObj); 
const GUI_FONT * TEXT_GetFont     (TEXT_Handle hObj);
int              TEXT_GetNumLines (TEXT_Handle hObj);
int              TEXT_GetText     (TEXT_Handle hObj, char * pDest, U32 BufferSize);
int              TEXT_GetTextAlign(TEXT_Handle hObj);
GUI_COLOR        TEXT_GetTextColor(TEXT_Handle hObj);
int              TEXT_GetUserData (TEXT_Handle hObj, void * pDest, int NumBytes);
GUI_WRAPMODE     TEXT_GetWrapMode (TEXT_Handle hObj);
void             TEXT_SetBkColor  (TEXT_Handle hObj, GUI_COLOR Color);
void             TEXT_SetFont     (TEXT_Handle hObj, const GUI_FONT * pFont);
int              TEXT_SetText     (TEXT_Handle hObj, const char * s);
void             TEXT_SetTextAlign(TEXT_Handle hObj, int Align);
void             TEXT_SetTextColor(TEXT_Handle hObj, GUI_COLOR Color);
int              TEXT_SetUserData (TEXT_Handle hObj, const void * pSrc, int NumBytes);
void             TEXT_SetWrapMode (TEXT_Handle hObj, GUI_WRAPMODE WrapMode);

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

const GUI_FONT * TEXT_GetDefaultFont     (void);
GUI_COLOR        TEXT_GetDefaultTextColor(void);
GUI_WRAPMODE     TEXT_GetDefaultWrapMode (void);
void             TEXT_SetDefaultFont     (const GUI_FONT * pFont);
void             TEXT_SetDefaultTextColor(GUI_COLOR Color);
GUI_WRAPMODE     TEXT_SetDefaultWrapMode (GUI_WRAPMODE WrapMode);

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // TEXT_H

/*************************** End of file ****************************/
