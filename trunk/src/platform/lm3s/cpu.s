//*****************************************************************************
//
// cpu.S - Instruction wrappers for special CPU instructions needed by the
//         drivers.
//
// Copyright (c) 2006-2008 Luminary Micro, Inc.  All rights reserved.
// 
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2752 of the Stellaris Peripheral Driver Library.
//
//*****************************************************************************

#include "asmdefs.h"

//*****************************************************************************
//
// The entire contents of this file go into the code section.
//
//*****************************************************************************
    __LIBRARY__ __lib_cpu
    __TEXT_NOROOT__

//*****************************************************************************
//
// Wrapper function for the CPSID instruction.  Returns the state of PRIMASK
// on entry.
//
//*****************************************************************************
    __EXPORT__ CPUcpsid
    __THUMB_LABEL__
CPUcpsid __LABEL__
    mrs     r0, PRIMASK
    cpsid   i
    bx      lr

//*****************************************************************************
//
// Wrapper function for the CPSIE instruction.  Returns the state of PRIMASK
// on entry.
//
//*****************************************************************************
    __EXPORT__ CPUcpsie
    __THUMB_LABEL__
CPUcpsie __LABEL__
    mrs     r0, PRIMASK
    cpsie   i
    bx      lr

//*****************************************************************************
//
// Wrapper function for the WFI instruction.
//
//*****************************************************************************
    __EXPORT__ CPUwfi
    __THUMB_LABEL__
CPUwfi __LABEL__
    wfi
    bx      lr
