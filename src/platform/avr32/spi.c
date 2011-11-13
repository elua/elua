#include "compiler.h"
#include "spi.h"

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


/*! \name SPI Writable Bit-Field Registers
 */
//! @{
typedef union
{
  unsigned long                 cr;
  avr32_spi_cr_t                CR;
} u_avr32_spi_cr_t;

typedef union
{
  unsigned long                 mr;
  avr32_spi_mr_t                MR;
} u_avr32_spi_mr_t;

typedef union
{
  unsigned long                 tdr;
  avr32_spi_tdr_t               TDR;
} u_avr32_spi_tdr_t;

typedef union
{
  unsigned long                 ier;
  avr32_spi_ier_t               IER;
} u_avr32_spi_ier_t;

typedef union
{
  unsigned long                 idr;
  avr32_spi_idr_t               IDR;
} u_avr32_spi_idr_t;

typedef union
{
  unsigned long                 csr;
  avr32_spi_csr0_t              CSR;
} u_avr32_spi_csr_t;
//! @}
/*-----------------------------------------------------------*/
void spi_reset(volatile avr32_spi_t *spi)
{
  spi->cr = AVR32_SPI_CR_SWRST_MASK;
}
/*-----------------------------------------------------------*/
int spi_initMaster(volatile avr32_spi_t *spi, const spi_master_options_t *opt, U32 pba_hz)
{
  u_avr32_spi_mr_t u_avr32_spi_mr;

  // Reset.
  spi->cr = AVR32_SPI_CR_SWRST_MASK;

  // Master Mode.
  // The mode register's reset state is all 0s, so omit 0 values.
  u_avr32_spi_mr.mr = spi->mr;
  u_avr32_spi_mr.MR.dlybcs = opt->delay*(pba_hz/1000000UL);
  u_avr32_spi_mr.MR.pcs = (1 << AVR32_SPI_MR_PCS_SIZE) - 1;
  //u_avr32_spi_mr.MR.llb = 0;
  u_avr32_spi_mr.MR.modfdis = opt->modfdis;
  //u_avr32_spi_mr.MR.fdiv = 0;
  u_avr32_spi_mr.MR.pcsdec = opt->pcs_decode;
  //u_avr32_spi_mr.MR.ps = 0;
  u_avr32_spi_mr.MR.mstr = 1;
  spi->mr = u_avr32_spi_mr.mr;

  // SPI enable
  spi->cr = AVR32_SPI_CR_SPIEN_MASK;


  return 0;
}
/*-----------------------------------------------------------*/
// Changed for eLua not to set clock frequencies higher than requested
// and to return the actual baud rate that was set.
U32 spi_setupChipReg(volatile avr32_spi_t *spi,
                     unsigned char reg, const spi_options_t *options, U32 pba_hz)
{
  u_avr32_spi_csr_t u_avr32_spi_csr;
  U32 baudDiv;

  if (options->mode > 3 ||
      options->bits < 8 || options->bits > 16) {
    return 0;
  }

  // Use a frequency less than or equal to that requested, not the nearest
  // available one, to avoid driving devices over their maximum speeds.
  // A frequency less than or equal needs a divisor greater than or equal,
  // and this formula cannot give a result of 0, so no need to check for it.
  // Well, unless pba_hz is passed as 0...
  if (options->baudrate == 0) baudDiv = 255;
  else {
    baudDiv = (pba_hz + options->baudrate - 1) / options->baudrate;
    if (baudDiv > 255) baudDiv = 255;
  }

  // Will use CSR0 offsets; these are the same for CSR0 to CSR3.
  u_avr32_spi_csr.csr = 0;
  u_avr32_spi_csr.CSR.cpol = options->mode >> 1;
  u_avr32_spi_csr.CSR.ncpha = (options->mode & 0x1) ^ 0x1;
  u_avr32_spi_csr.CSR.csaat = 1;
  u_avr32_spi_csr.CSR.bits = options->bits - 8;
  u_avr32_spi_csr.CSR.scbr = baudDiv;
  u_avr32_spi_csr.CSR.dlybs = min(options->spck_delay*(pba_hz/1000000UL), 255);
  u_avr32_spi_csr.CSR.dlybct = min((options->trans_delay*(pba_hz/1000000UL) + 31)/32, 255);

  switch(reg) {
    case 0:
      spi->csr0 = u_avr32_spi_csr.csr;
      break;
    case 1:
      spi->csr1 = u_avr32_spi_csr.csr;
      break;
    case 2:
      spi->csr2 = u_avr32_spi_csr.csr;
      break;
    case 3:
      spi->csr3 = u_avr32_spi_csr.csr;
      break;
    default:
      return 0;  // Cannot happen in eLua
  }

  // Return the nearest integer to the actual baud rate
  return (pba_hz + baudDiv/2) / baudDiv;
}
/*-----------------------------------------------------------*/
int spi_selectChip(volatile avr32_spi_t *spi, unsigned char chip)
{
  // Assert all lines; no peripheral is selected.
  spi->mr |= AVR32_SPI_MR_PCS_MASK;

  if (spi->mr & AVR32_SPI_MR_PCSDEC_MASK) {
    // The signal is decoded; allow up to 15 chips.
    if (chip > 14) goto err;
    spi->mr &= ~AVR32_SPI_MR_PCS_MASK | (chip << AVR32_SPI_MR_PCS_OFFSET);
  } else {
    if (chip > 3) goto err;
    spi->mr &= ~(1 << (AVR32_SPI_MR_PCS_OFFSET + chip));
  }

  return 0;
err:
  return -1;
}
/*-----------------------------------------------------------*/
int spi_unselectChip(volatile avr32_spi_t *spi, unsigned char chip)
{
  while (!(spi->sr & AVR32_SPI_SR_TXEMPTY_MASK))
    continue;

  // Assert all lines; no peripheral is selected.
  spi->mr |= AVR32_SPI_MR_PCS_MASK;

  // Last transfer, so deassert the current NPCS if CSAAT is set.
  spi->cr = AVR32_SPI_CR_LASTXFER_MASK;

  return 0;
}
//! Time-out value (number of attempts).
#define SPI_TIMEOUT       10000
/*-----------------------------------------------------------*/
U16 spi_single_transfer(volatile avr32_spi_t *spi, U16 txdata)
{
  U16 ret;
  unsigned int timeout = SPI_TIMEOUT;

  /* Wait for any pending TX */
  while (!(spi->sr & AVR32_SPI_SR_TDRE_MASK)) {
    if (!timeout--) {
      return 0XFF;
    }
  }
  // Discard data in buffer if any
  ret = (spi->rdr >> AVR32_SPI_RDR_RD_OFFSET);

  spi->tdr = txdata << AVR32_SPI_TDR_TD_OFFSET;

  while ((spi->sr & (AVR32_SPI_SR_RDRF_MASK | AVR32_SPI_SR_TXEMPTY_MASK)) !=
         (AVR32_SPI_SR_RDRF_MASK | AVR32_SPI_SR_TXEMPTY_MASK)) {
    if (!timeout--) {
      return 0xFF;
    }
  }


  ret = (spi->rdr >> AVR32_SPI_RDR_RD_OFFSET);

  return ret;
}
/*-----------------------------------------------------------*/


