//*****************************************************************************
//
// usbtick.c - Functions related to USB stack tick timer handling.
//
// Copyright (c) 2008-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 7611 of the Stellaris USB Library.
//
//*****************************************************************************

#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "usblib/usblib.h"
#include "usblib/usblibpriv.h"

//*****************************************************************************
//
//! \addtogroup general_usblib_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// These are the internal timer tick handlers used by the USB stack.  Handlers
// in g_pfTickHandlers are called in the context of the USB SOF interrupt
// every USB_SOF_TICK_DIVIDE milliseconds.
//
//*****************************************************************************
tUSBTickHandler g_pfTickHandlers[USB_TICK_HANDLER_NUM];
void *g_pvTickInstance[USB_TICK_HANDLER_NUM];

//*****************************************************************************
//
// Flag to indicate whether or not we have been initialized.
//
//*****************************************************************************
tBoolean g_bUSBTimerInitialized = false;

//*****************************************************************************
//
// This is the current tick value in ms for the system.  This is used for all
// instances of USB controllers and for all timer tick handlers.
//
//*****************************************************************************
unsigned long g_ulCurrentUSBTick = 0;

//*****************************************************************************
//
// This is the total number of SOF interrupts received since the system
// booted.  The value is incremented by the low level device- or host-interrupt
// handler functions.
//
//*****************************************************************************
unsigned long g_ulUSBSOFCount = 0;

//*****************************************************************************
//
// This internal function initializes the variables used in processing timer
// ticks.
//
// This function should only be called from within the USB library.  It is set
// up to ensure that it can be called multiple times if necessary without
// the previous configuration being erased (to cater for OTG mode switching).
//
// \return None.
//
//*****************************************************************************
void
InternalUSBTickInit(void)
{
    unsigned long ulLoop;

    if(!g_bUSBTimerInitialized)
    {
        for(ulLoop = 0; ulLoop < USB_TICK_HANDLER_NUM; ulLoop++)
        {
            g_pfTickHandlers[ulLoop] = (tUSBTickHandler)0;
            g_pvTickInstance[ulLoop] = 0;
        }

        g_bUSBTimerInitialized = true;
    }
}

//*****************************************************************************
//
// This internal function handles registering OTG, Host, or Device SOF timer
// handler functions.
//
// \param ulHandler specifies which type of handler to register.
// \param pfHandler specifies the handler to call for the given type of
// handler.
//
// This function should only be called inside the USB library and only as a
// result to a call to reinitialize the stack in a new mode.  Currently the
// following 3 types of timer tick handlers can be registered:
// TICK_HANDLER_OTG, TICK_HANDLER_HOST, or TICK_HANDLER_DEVICE.  Handlers
// registered via this function are called in the context of the SOF interrupt.
//
// \return None.
//
//*****************************************************************************
void
InternalUSBRegisterTickHandler(unsigned long ulHandler,
                               tUSBTickHandler pfHandler,
                               void *pvInstance)
{
    ASSERT(ulHandler < USB_TICK_HANDLER_NUM);

    //
    // Save the handler.
    //
    g_pfTickHandlers[ulHandler] = pfHandler;

    //
    // Save the instance data.
    //
    g_pvTickInstance[ulHandler] = pvInstance;
}

//*****************************************************************************
//
//! \internal
//!
//! Calls internal handlers in response to a tick based on the start of frame
//! interrupt.
//!
//! \param ulTicksmS specifies how many milliseconds have passed since the last
//! call to this function.
//!
//! This function is called every 5mS in the context of the Start of Frame
//! (SOF) interrupt.  It is used to call any registered internal tick
//! functions.
//!
//! This function should only be called from within the USB library.
//!
//! \return None.
//
//*****************************************************************************
void
InternalUSBStartOfFrameTick(unsigned long ulTicksmS)
{
    long lIdx;

    //
    // Advance time.
    //
    g_ulCurrentUSBTick += ulTicksmS;

    //
    // Call any registered SOF tick handlers.
    //
    for(lIdx = 0; lIdx < USB_TICK_HANDLER_NUM; lIdx++)
    {
        if(g_pfTickHandlers[lIdx])
        {
            g_pfTickHandlers[lIdx](g_pvTickInstance[lIdx], ulTicksmS);
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
