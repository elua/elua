/* This file is part of the ATMEL AVR32-SoftwareFramework-1.3.0-AT32UC3A Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief AVR32UC C runtime startup file.
 *
 * This file has been built from the Newlib crt0.S.
 *
 * - Compiler:           GNU GCC for AVR32
 * - Supported devices:  All AVR32UC devices can be used.
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (C) 2006-2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <avr32/io.h>

//! @{
//! \verbatim


  // crt0 is placed outside the .reset section so that the program entry point
  // can be changed without affecting the C runtime startup.
  .section  .start, "ax", @progbits


  .global crt0
  .type crt0, @function
crt0:
  // Set initial stack pointer.
  lda.w   sp, _sstack

  // Set up EVBA so interrupts can be enabled.
  lda.w   r0, _evba
  mtsr    AVR32_EVBA, r0

  // Enable the exception processing.
  csrf    AVR32_SR_EM_OFFSET

  // Load initialized data having a global lifetime from the data LMA.
  lda.w   r0, _data
  lda.w   r1, _edata
  cp      r0, r1
  brhs    idata_load_loop_end
  lda.w   r2, _etext
idata_load_loop:
  ld.w    r4, r2++
  st.w    r0++, r4
  cp      r0, r1
  brlo    idata_load_loop
idata_load_loop_end:

  // Clear uninitialized data having a global lifetime in the blank static storage section.
  lda.w   r0, _bss
  lda.w   r1, _ebss
  cp      r0, r1
  brhs    udata_clear_loop_end
  mov     r2, 0
  mov     r3, 0
udata_clear_loop:
  st.w    r0++, r2
  cp      r0, r1
  brlo    udata_clear_loop
udata_clear_loop_end:

  // Safety: Set the default "return" @ to the exit routine address.
  lda.w   lr, _exit

  // Start the show.
  lda.w   pc, main

_exit:
  bral    _exit

//! \endverbatim
//! @}
