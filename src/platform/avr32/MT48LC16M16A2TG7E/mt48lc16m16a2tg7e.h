/* This header file is part of the ATMEL AVR-UC3-SoftwareFramework-1.6.1 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief MT48LC16M16A2TG-7E SDRAM driver for AVR32 UC3 SDRAMC on EBI.
 *
 * \note The values defined in this file are device-specific. See the device
 *       datasheet for further information.
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

#ifndef _MT48LC16M16A2TG7E_H_
#define _MT48LC16M16A2TG7E_H_


//! The number of bank bits for this SDRAM (1 or 2).
#define SDRAM_BANK_BITS                 2

//! The number of row bits for this SDRAM (11 to 13).
#define SDRAM_ROW_BITS                  13

//! The number of column bits for this SDRAM (8 to 11).
#define SDRAM_COL_BITS                  9

//! The minimal column address select (READ) latency for this SDRAM (1 to 3 SDRAM cycles).
//! Unit: tCK (SDRAM cycle period).
#define SDRAM_CAS                       2

//! The minimal write recovery time for this SDRAM (0 to 15 SDRAM cycles).
//! Unit: ns.
#define SDRAM_TWR                       14

//! The minimal row cycle time for this SDRAM (0 to 15 SDRAM cycles).
//! ACTIVE-to-ACTIVE command delay.
//! Unit: ns.
#define SDRAM_TRC                       60

//! The minimal row precharge time for this SDRAM (0 to 15 SDRAM cycles).
//! PRECHARGE command period.
//! Unit: ns.
#define SDRAM_TRP                       15

//! The minimal row to column delay time for this SDRAM (0 to 15 SDRAM cycles).
//! ACTIVE-to-READ/WRITE command delay.
//! Unit: ns.
#define SDRAM_TRCD                      15

//! The minimal row address select time for this SDRAM (0 to 15 SDRAM cycles).
//! ACTIVE-to-PRECHARGE command delay.
//! Unit: ns.
#define SDRAM_TRAS                      37

//! The minimal exit self refresh time for this SDRAM (0 to 15 SDRAM cycles).
//! Exit SELF REFRESH to ACTIVE command delay.
//! Unit: ns.
#define SDRAM_TXSR                      67

//! The maximal refresh time for this SDRAM (0 to 4095 SDRAM cycles).
//! Refresh period.
//! Unit: ns.
#define SDRAM_TR                        7812

//! The minimal refresh cycle time for this SDRAM.
//! AUTO REFRESH command period.
//! Unit: ns.
#define SDRAM_TRFC                      66

//! The minimal mode register delay time for this SDRAM.
//! LOAD MODE REGISTER command to ACTIVE or REFRESH command delay.
//! Unit: tCK (SDRAM cycle period).
#define SDRAM_TMRD                      2

//! The minimal stable-clock initialization delay for this SDRAM.
//! Unit: us.
#define SDRAM_STABLE_CLOCK_INIT_DELAY   100

//! The minimal number of AUTO REFRESH commands required during initialization for this SDRAM.
#define SDRAM_INIT_AUTO_REFRESH_COUNT   2


#endif  // _MT48LC16M16A2TG7E_H_
