#ifndef _SPI_H_
#define _SPI_H_

#include <avr32/io.h>

#ifndef AVR32_SPI0
    #define AVR32_SPI0          AVR32_SPI
    #define AVR32_SPI0_ADDRESS  AVR32_SPI_ADDRESS
#endif

typedef enum {
  SPI_MODE_0 = 0,
  SPI_MODE_1,
  SPI_MODE_2,
  SPI_MODE_3
} spi_mode_t;

//! Option structure for SPI channels.
typedef struct
{
  //! Preferred baudrate for the SPI.
  unsigned int baudrate;

  //! Number of bits in each character (8 to 16).
  unsigned char bits;

  //! Delay before first clock pulse after selecting slave (in microseconds).
  unsigned int spck_delay;

  //! Delay between each transfer/character (in microseconds).
  unsigned int trans_delay;

  //! Which SPI mode to use when transmitting.
  spi_mode_t mode;

} spi_options_t;

typedef struct
{
  //! Mode fault detection disable
  Bool modfdis;
  //! Chip select decoding
  Bool pcs_decode;
  //! delay before chip select (in microseconds)
  unsigned int delay;
} spi_master_options_t;


extern void spi_reset(volatile avr32_spi_t *spi);

extern int spi_initMaster(volatile avr32_spi_t *spi, const spi_master_options_t *, U32 pba_hz);
extern U32 spi_setupChipReg(volatile avr32_spi_t *spi, unsigned char chip, const spi_options_t *, U32 pba_hz);

extern int spi_selectChip(volatile avr32_spi_t *spi, unsigned char chip);
extern int spi_unselectChip(volatile avr32_spi_t *spi, unsigned char chip);

extern U16 spi_single_transfer(volatile avr32_spi_t *spi, U16 txdata);

#endif  // _SPI_H_
