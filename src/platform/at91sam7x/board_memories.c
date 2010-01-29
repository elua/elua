/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "board_memories.h"
#include "board.h"

//------------------------------------------------------------------------------
//         Internal definitions
//------------------------------------------------------------------------------
/*
    Constants: Remap types
        BOARD_FLASH - Flash is mirrored in the remap zone.
        BOARD_RAM - RAM is mirrored in the remap zone.
*/
#define BOARD_FLASH             0
#define BOARD_RAM               1

//------------------------------------------------------------------------------
//         Internal function
//------------------------------------------------------------------------------
/*
    Function: BOARD_GetRemap
        Returns the current remap (see <Remap types>).
*/
static unsigned char BOARD_GetRemap( void )
{
    unsigned int *remap = (unsigned int *) 0;
    unsigned int *ram = (unsigned int *) AT91C_ISRAM;

    // Try to write in 0 and see if this affects the RAM
    unsigned int temp = *ram;
    *ram = temp + 1;
    if (*remap == *ram) {

        *ram = temp;
        return BOARD_RAM;
    }
    else {

        *ram = temp;
        return BOARD_FLASH;
    }
}

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------
/*
    Function: BOARD_RemapFlash
        Changes the mapping of the chip so that the remap area mirrors the
        internal flash.
*/
void BOARD_RemapFlash( void )
{
    if (BOARD_GetRemap() != BOARD_FLASH) {

        AT91C_BASE_MC->MC_RCR = AT91C_MC_RCB;
    }
}

/*
    Function: BOARD_RemapRam
        Changes the mapping of the chip so that the remap area mirrors the
        internal RAM.
*/
void BOARD_RemapRam( void )
{
    if (BOARD_GetRemap() != BOARD_RAM) {

        AT91C_BASE_MC->MC_RCR = AT91C_MC_RCB;
    }
}

