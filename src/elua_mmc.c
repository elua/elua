/*-----------------------------------------------------------------------*/
/* MMC/SDC (in SPI mode) control module  (C)ChaN, 2007                   */
/*-----------------------------------------------------------------------*/

// This file was modified from a sample available from the FatFs
// web site by Jesus Alvarez & James Snyder for eLua.
 
#include "platform_conf.h"
#if defined( BUILD_MMCFS ) && !defined( ELUA_SIMULATOR ) && !defined( XMC4500_F144x1024 ) && !defined( XMC4500_E144x1024 ) && !defined( XMC4700_F144x2048 )
#include "platform.h"
#include "diskio.h"
#include "mmcfs.h"

#ifndef MMCFS_NUM_CARDS
#define NUM_CARDS             1
#else
#define NUM_CARDS             MMCFS_NUM_CARDS
#endif

/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)    /* GO_IDLE_STATE */
#define CMD1    (0x40+1)    /* SEND_OP_COND */
#define CMD8    (0x40+8)    /* SEND_IF_COND */
#define CMD9    (0x40+9)    /* SEND_CSD */
#define CMD10    (0x40+10)    /* SEND_CID */
#define CMD12    (0x40+12)    /* STOP_TRANSMISSION */
#define CMD16    (0x40+16)    /* SET_BLOCKLEN */
#define CMD17    (0x40+17)    /* READ_SINGLE_BLOCK */
#define CMD18    (0x40+18)    /* READ_MULTIPLE_BLOCK */
#define CMD23    (0x40+23)    /* SET_BLOCK_COUNT */
#define CMD24    (0x40+24)    /* WRITE_BLOCK */
#define CMD25    (0x40+25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41    (0x40+41)    /* SEND_OP_COND (ACMD) */
#define CMD55    (0x40+55)    /* APP_CMD */
#define CMD58    (0x40+58)    /* READ_OCR */

#if !defined( MMCFS_SPI_NUM ) && !defined( MMCFS_SPI_NUM_ARRAY )
  #error "MMC not supported on this board"
#endif

#if defined( MMCFS_CS_PORT )
const u8 mmcfs_cs_ports[ NUM_CARDS ] = { MMCFS_CS_PORT };
static const u8 mmcfs_cs_pins[ NUM_CARDS ] = { MMCFS_CS_PIN };
static const u8 mmcfs_spi_nums[ NUM_CARDS ] = { MMCFS_SPI_NUM };
#elif defined( MMCFS_CS_PORT_ARRAY )
const u8 mmcfs_cs_ports[ NUM_CARDS ] = MMCFS_CS_PORT_ARRAY;
static const u8 mmcfs_cs_pins[ NUM_CARDS ] = MMCFS_CS_PIN_ARRAY;
static const u8 mmcfs_spi_nums[ NUM_CARDS ] = MMCFS_SPI_NUM_ARRAY;
#endif

// asserts the CS pin to the card
static
void SELECT (BYTE id)
{
    platform_pio_op( mmcfs_cs_ports[ id ] , ( ( u32 ) 1 << mmcfs_cs_pins[ id ] ), PLATFORM_IO_PIN_CLEAR );    
}

// de-asserts the CS pin to the card
static
void DESELECT (BYTE id)
{
    platform_pio_op( mmcfs_cs_ports[ id ], ( ( u32 ) 1 << mmcfs_cs_pins[ id ] ), PLATFORM_IO_PIN_SET );
}


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile DSTATUS Stat[ NUM_CARDS ];    /* Disk status */

static BYTE TriesLeft[ NUM_CARDS ];

static BYTE CardType[ NUM_CARDS ];            /* b0:MMC, b1:SDC, b2:Block addressing */

static BYTE PowerFlag[ NUM_CARDS ];     /* indicates if "power" is on */


/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/


static
void xmit_spi (BYTE id, BYTE dat)
{
    platform_spi_send_recv( mmcfs_spi_nums[ id ], dat );
}


/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

static
BYTE rcvr_spi (BYTE id)
{
    DWORD rcvdat;

    rcvdat  = platform_spi_send_recv( mmcfs_spi_nums[ id ], 0xFF );

    return ( BYTE )rcvdat;
}


static
void rcvr_spi_m (BYTE id, BYTE *dst)
{
    *dst = rcvr_spi( id );
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
BYTE wait_ready (BYTE id)
{
    BYTE res;
    timer_data_type Timer2;

    Timer2 = platform_timer_read( PLATFORM_TIMER_SYS_ID );
    rcvr_spi( id );
    do  
        res = rcvr_spi( id ); /* Wait for ready in timeout of 500ms. */
    while ( ( res != 0xFF ) && ( platform_timer_get_diff_crt( PLATFORM_TIMER_SYS_ID, Timer2 ) < 500000 ) );

    return res;
}

/*-----------------------------------------------------------------------*/
/* Send 80 or so clock transitions with CS and DI held high. This is     */
/* required after card power up to get it into SPI mode                  */
/*-----------------------------------------------------------------------*/
static
void send_initial_clock_train(BYTE id)
{
    unsigned int i;
    /* Ensure CS is held high. */
    DESELECT(id);
    
    /* Send 10 bytes over the SSI. This causes the clock to wiggle the */
    /* required number of times. */
    for(i = 0 ; i < 10 ; i++)
        rcvr_spi(id);
}

/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (BYTE id)
{
    /*
     * This doesn't really turn the power on, but initializes the
     * SSI port and pins needed to talk to the card.
     */
    
    // Setup CS pin & deselect
    platform_pio_op( mmcfs_cs_ports[ id ], ( ( u32 ) 1 << mmcfs_cs_pins[ id ] ), PLATFORM_IO_PIN_DIR_OUTPUT );
    //platform_pio_op( MMCFS_CS_PORT, ( ( u32 ) 1 << MMCFS_CS_PIN ), PLATFORM_IO_PIN_PULLUP );
    DESELECT( id );
    
    // Setup SPI
    platform_spi_setup( mmcfs_spi_nums[ id ], PLATFORM_SPI_MASTER, 400000, 0, 0, 8 );

    /* Set DI and CS high and apply more than 74 pulses to SCLK for the card */
    /* to be able to accept a native command. */
    send_initial_clock_train(id);

    PowerFlag[id] = 1;
}

// set the SSI speed to the max setting
static
void set_max_speed(BYTE id)
{
    unsigned long i;

    /* Set the maximum speed as half the system clock, with a max of 12.5 MHz. */
    i = platform_cpu_get_frequency() / 2;
    if(i > 12500000)
        i = 12500000;

    /* Configure the SPI port */
    platform_spi_setup( mmcfs_spi_nums[ id ], PLATFORM_SPI_MASTER, i, 0, 0, 8 );
}

static
void power_off (BYTE id)
{
    PowerFlag[ id ] = 0;
}

static
int chk_power(BYTE id)        /* Socket power state: 0=off, 1=on */
{
    return PowerFlag[ id ];
}


/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
BOOL rcvr_datablock (
    BYTE id,            /* Disk ID */
    BYTE *buff,         /* Data buffer to store received data */
    UINT btr            /* Byte count (must be even number) */
)
{
    BYTE token;
    timer_data_type Timer1;

    Timer1 = platform_timer_read( PLATFORM_TIMER_SYS_ID );
    do {                            /* Wait for data packet in timeout of 100ms */
        token = rcvr_spi(id);
    } while ( ( token == 0xFF ) && 
              platform_timer_get_diff_crt( PLATFORM_TIMER_SYS_ID, Timer1 ) < 100000 );
    if(token != 0xFE) return FALSE;    /* If not valid data token, retutn with error */

    do {                            /* Receive the data block into buffer */
        rcvr_spi_m(id, buff++);
        rcvr_spi_m(id, buff++);
    } while (btr -= 2);
    rcvr_spi(id);                        /* Discard CRC */
    rcvr_spi(id);

    return TRUE;                    /* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
BOOL xmit_datablock (
    BYTE id,              /* Disk ID */
    const BYTE *buff,     /* 512 byte data block to be transmitted */
    BYTE token            /* Data/Stop token */
)
{
    BYTE resp, wc;


    if (wait_ready(id) != 0xFF) return FALSE;

    xmit_spi(id,token);                    /* Xmit data token */
    if (token != 0xFD) {    /* Is data token */
        wc = 0;
        do {                            /* Xmit the 512 byte data block to MMC */
            xmit_spi(id,*buff++);
            xmit_spi(id,*buff++);
        } while (--wc);
        xmit_spi(id,0xFF);                    /* CRC (Dummy) */
        xmit_spi(id,0xFF);
        resp = rcvr_spi(id);                /* Reveive data response */
        if ((resp & 0x1F) != 0x05)        /* If not accepted, return with error */
            return FALSE;
    }

    return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
    BYTE id,         /* Disk ID */
    BYTE cmd,        /* Command byte */
    DWORD arg        /* Argument */
)
{
    BYTE n, res;


    if (wait_ready(id) != 0xFF) return 0xFF;

    /* Send command packet */
    xmit_spi(id,cmd);                        /* Command */
    xmit_spi(id,(BYTE)(arg >> 24));        /* Argument[31..24] */
    xmit_spi(id,(BYTE)(arg >> 16));        /* Argument[23..16] */
    xmit_spi(id,(BYTE)(arg >> 8));            /* Argument[15..8] */
    xmit_spi(id,(BYTE)arg);                /* Argument[7..0] */
    n = 0;
    if (cmd == CMD0) n = 0x95;            /* CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;            /* CRC for CMD8(0x1AA) */
    xmit_spi(id,n);

    /* Receive command response */
    if (cmd == CMD12) rcvr_spi(id);        /* Skip a stuff byte when stop reading */
    n = 10;                                /* Wait for a valid response in timeout of 10 attempts */
    do
        res = rcvr_spi(id);
    while ((res & 0x80) && --n);

    return res;            /* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

void elua_mmc_init( void )
{
  int i;

  for( i = 0; i < NUM_CARDS; i ++ )
  {
    Stat[ i ] = STA_NOINIT;
    TriesLeft[ i ] = 2;
  }
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE drv        /* Physical drive nmuber (0) */
)
{
    BYTE n, ty, ocr[4];
    timer_data_type Timer1;

    if (Stat[drv] & STA_NODISK) return Stat[drv];    /* No card in the socket */
    
    do
    {
      power_on(drv);                           /* Force socket power on */

      SELECT(drv);                /* CS = L */
      ty = 0;
      if (send_cmd(drv,CMD0, 0) == 1) {            /* Enter Idle state */
        Timer1 = platform_timer_read( PLATFORM_TIMER_SYS_ID );
        if (send_cmd(drv,CMD8, 0x1AA) == 1) {    /* SDC Ver2+ */
          for (n = 0; n < 4; n++) ocr[n] = rcvr_spi(drv);
          if (ocr[2] == 0x01 && ocr[3] == 0xAA) {    /* The card can work at vdd range of 2.7-3.6V */
            do {
              if (send_cmd(drv,CMD55, 0) <= 1 && send_cmd(drv,CMD41, 1UL << 30) == 0)    break;    /* ACMD41 with HCS bit */
            } while ( platform_timer_get_diff_crt( PLATFORM_TIMER_SYS_ID, Timer1 ) < 1000000 );
            if ( ( platform_timer_get_diff_crt( PLATFORM_TIMER_SYS_ID, Timer1 ) < 1000000 ) 
                 && send_cmd(drv,CMD58, 0) == 0) {    /* Check CCS bit (it seems pointless to check the timer here*/
              for (n = 0; n < 4; n++) ocr[n] = rcvr_spi(drv);
              ty = (ocr[0] & 0x40) ? 6 : 2;
            }
          }
        } else {                            /* SDC Ver1 or MMC */
          ty = (send_cmd(drv,CMD55, 0) <= 1 && send_cmd(drv,CMD41, 0) <= 1) ? 2 : 1;    /* SDC : MMC */
          do {
            if (ty == 2) {
              if (send_cmd(drv,CMD55, 0) <= 1 && send_cmd(drv,CMD41, 0) == 0) break;    /* ACMD41 */
            } else {
              if (send_cmd(drv,CMD1, 0) == 0) break;                                /* CMD1 */
            }
          } while ( platform_timer_get_diff_crt( PLATFORM_TIMER_SYS_ID, Timer1 ) < 1000000 );
          if ( (  platform_timer_get_diff_crt( PLATFORM_TIMER_SYS_ID, Timer1 ) >= 1000000 ) 
               || send_cmd(drv,CMD16, 512) != 0 )    /* Select R/W block length */
            ty = 0;
        }
      }
      CardType[drv] = ty;
      DESELECT(drv);            /* CS = H */
      rcvr_spi(drv);            /* Idle (Release DO) */

      if (TriesLeft[drv])
        TriesLeft[drv]--;

      if (ty) {            /* Initialization succeded */
        Stat[drv] &= ~STA_NOINIT;        /* Clear STA_NOINIT */
        set_max_speed(drv);
      } else {            /* Initialization failed */
        power_off(drv);
      }

    } while( TriesLeft[drv] > 0 && ty == 0 );

    return Stat[drv];
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE drv        /* Physical drive nmuber (0) */
)
{
    return Stat[drv];
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE drv,            /* Physical drive nmuber (0) */
    BYTE *buff,            /* Pointer to the data buffer to store read data */
    DWORD sector,        /* Start sector number (LBA) */
    BYTE count            /* Sector count (1..255) */
)
{
    if (!count) return RES_PARERR;
    if (Stat[drv] & STA_NOINIT) return RES_NOTRDY;

    if (!(CardType[drv] & 4)) sector *= 512;    /* Convert to byte address if needed */

    SELECT(drv);            /* CS = L */

    if (count == 1) {    /* Single block read */
        if ((send_cmd(drv,CMD17, sector) == 0)    /* READ_SINGLE_BLOCK */
            && rcvr_datablock(drv,buff, 512))
            count = 0;
    }
    else {                /* Multiple block read */
        if (send_cmd(drv,CMD18, sector) == 0) {    /* READ_MULTIPLE_BLOCK */
            do {
                if (!rcvr_datablock(drv,buff, 512)) break;
                buff += 512;
            } while (--count);
            send_cmd(drv,CMD12, 0);                /* STOP_TRANSMISSION */
        }
    }

    DESELECT(drv);            /* CS = H */
    rcvr_spi(drv);            /* Idle (Release DO) */

    return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
    BYTE drv,            /* Physical drive nmuber (0) */
    const BYTE *buff,    /* Pointer to the data to be written */
    DWORD sector,        /* Start sector number (LBA) */
    BYTE count            /* Sector count (1..255) */
)
{
    if (!count) return RES_PARERR;
    if (Stat[drv] & STA_NOINIT) return RES_NOTRDY;
    if (Stat[drv] & STA_PROTECT) return RES_WRPRT;

    if (!(CardType[drv] & 4)) sector *= 512;    /* Convert to byte address if needed */

    SELECT(drv);            /* CS = L */

    if (count == 1) {    /* Single block write */
        if ((send_cmd(drv, CMD24, sector) == 0)    /* WRITE_BLOCK */
            && xmit_datablock(drv, buff, 0xFE))
            count = 0;
    }
    else {                /* Multiple block write */
        if (CardType[drv] & 2) {
            send_cmd(drv, CMD55, 0); send_cmd(drv, CMD23, count);    /* ACMD23 */
        }
        if (send_cmd(drv, CMD25, sector) == 0) {    /* WRITE_MULTIPLE_BLOCK */
            do {
                if (!xmit_datablock(drv, buff, 0xFC)) break;
                buff += 512;
            } while (--count);
            if (!xmit_datablock(drv, 0, 0xFD))    /* STOP_TRAN token */
                count = 1;
        }
    }

    DESELECT(drv);            /* CS = H */
    rcvr_spi(drv);            /* Idle (Release DO) */

    return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE drv,        /* Physical drive nmuber (0) */
    BYTE ctrl,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    DRESULT res;
    BYTE n, csd[16], *ptr = buff;
    WORD csize;


    res = RES_ERROR;

    if (ctrl == CTRL_POWER) {
        switch (*ptr) {
        case 0:        /* Sub control code == 0 (POWER_OFF) */
            if (chk_power(drv))
                power_off(drv);        /* Power off */
            res = RES_OK;
            break;
        case 1:        /* Sub control code == 1 (POWER_ON) */
            power_on(drv);                /* Power on */
            res = RES_OK;
            break;
        case 2:        /* Sub control code == 2 (POWER_GET) */
            *(ptr+1) = (BYTE)chk_power(drv);
            res = RES_OK;
            break;
        default :
            res = RES_PARERR;
        }
    }
    else {
        if (Stat[drv] & STA_NOINIT) return RES_NOTRDY;

        SELECT(drv);        /* CS = L */

        switch (ctrl) {
        case GET_SECTOR_COUNT :    /* Get number of sectors on the disk (DWORD) */
            if ((send_cmd(drv,CMD9, 0) == 0) && rcvr_datablock(drv,csd, 16)) {
                if ((csd[0] >> 6) == 1) {    /* SDC ver 2.00 */
                    csize = csd[9] + ((WORD)csd[8] << 8) + 1;
                    *(DWORD*)buff = (DWORD)csize << 10;
                } else {                    /* MMC or SDC ver 1.XX */
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                    *(DWORD*)buff = (DWORD)csize << (n - 9);
                }
                res = RES_OK;
            }
            break;

        case GET_SECTOR_SIZE :    /* Get sectors on the disk (WORD) */
            *(WORD*)buff = 512;
            res = RES_OK;
            break;

        case CTRL_SYNC :    /* Make sure that data has been written */
            if (wait_ready(drv) == 0xFF)
                res = RES_OK;
            break;

        case MMC_GET_CSD :    /* Receive CSD as a data block (16 bytes) */
            if (send_cmd(drv,CMD9, 0) == 0        /* READ_CSD */
                && rcvr_datablock(drv, ptr, 16))
                res = RES_OK;
            break;

        case MMC_GET_CID :    /* Receive CID as a data block (16 bytes) */
            if (send_cmd(drv, CMD10, 0) == 0        /* READ_CID */
                && rcvr_datablock(drv, ptr, 16))
                res = RES_OK;
            break;

        case MMC_GET_OCR :    /* Receive OCR as an R3 resp (4 bytes) */
            if (send_cmd(drv, CMD58, 0) == 0) {    /* READ_OCR */
                for (n = 0; n < 4; n++)
                    *ptr++ = rcvr_spi(drv);
                res = RES_OK;
            }

//        case MMC_GET_TYPE :    /* Get card type flags (1 byte) */
//            *ptr = CardType;
//            res = RES_OK;
//            break;

        default:
            res = RES_PARERR;
        }

        DESELECT(drv);            /* CS = H */
        rcvr_spi(drv);            /* Idle (Release DO) */
    }

    return res;
}



/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */

DWORD get_fattime (void)
{

    return    ((2007UL-1980) << 25)    // Year = 2007
            | (6UL << 21)            // Month = June
            | (5UL << 16)            // Day = 5
            | (11U << 11)            // Hour = 11
            | (38U << 5)            // Min = 38
            | (0U >> 1)                // Sec = 0
            ;

}
#endif // #if defined( BUILD_MMCFS ) && !defined( ELUA_SIMULATOR )

