/* This header file is part of the ATMEL AVR-UC3-SoftwareFramework-1.6.1 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief SDRAMC on EBI driver for AVR32 UC3.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with an SDRAMC module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

#ifndef _SDRAMC_H_
#define _SDRAMC_H_

#include <avr32/io.h>
#include "board.h"

#ifdef SDRAM_PART_HDR
  #include SDRAM_PART_HDR
#else
  #error No SDRAM header file defined
#endif

//! Pointer to SDRAM.
#if BOARD == UC3C_EK
#define SDRAM           ((void *)AVR32_EBI_CS1_0_ADDRESS)
#else
#define SDRAM           ((void *)AVR32_EBI_CS1_ADDRESS)
#endif

//! SDRAM size.
#define SDRAM_SIZE      (1 << (SDRAM_BANK_BITS + \
                               SDRAM_ROW_BITS  + \
                               SDRAM_COL_BITS  + \
                               (SDRAM_DBW >> 4)))


/*! \brief Initializes the AVR32 SDRAM Controller and the connected SDRAM(s).
 *
 * \param hsb_hz HSB frequency in Hz (the HSB frequency is applied to the SDRAMC
 *               and to the SDRAM).
 *
 * \note HMATRIX and SDRAMC registers are always read with a dummy load
 *       operation after having been written to, in order to force write-back
 *       before executing the following accesses, which depend on the values set
 *       in these registers.
 *
 * \note Each access to the SDRAM address space validates the mode of the SDRAMC
 *       and generates an operation corresponding to this mode.
 */
extern void sdramc_init(unsigned long hsb_hz);


#endif  // _SDRAMC_H_
