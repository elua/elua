/*-----------------------------------------------------------------------*/
/* MMC/SD emulator for the eLua simulator                                */
/*-----------------------------------------------------------------------*/

#include "platform_conf.h"
#if defined( BUILD_MMCFS ) && defined( ELUA_SIMULATOR )
#include "platform.h"
#include "hostif.h"
#include "diskio.h"
#include <stdio.h>

#define SD_CARD_SIM_NAME                "sdcard.img"

/*--------------------------------------------------------------------------
   Module Private Functions
---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;    /* Disk status */

static
BYTE PowerFlag = 0;     /* indicates if "power" is on */

static int fd = -1;
static long imgsize;

static
void power_on (void)
{
  PowerFlag = 1;
}

static
void power_off (void)
{
  PowerFlag = 0;
}

static
int chk_power(void)        /* Socket power state: 0=off, 1=on */
{
  return PowerFlag;
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE drv        /* Physical drive nmuber (0) */
)
{
  if( drv )
    return STA_NOINIT;

  if( fd != -1 )
    return Stat;

  fd = hostif_open( SD_CARD_SIM_NAME, 2, 0666 );

  if( fd == -1 )
    printf( "[SDSIM] Unable to open " SD_CARD_SIM_NAME "\n" );
  else
  {
    Stat = 0;
    imgsize = hostif_lseek( fd, 0, SEEK_END );
    hostif_lseek( fd, 0, SEEK_SET );
    printf( "[SDSIM] found SD card image, size=%ld bytes\n", imgsize );
  }
  return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/


DSTATUS disk_status (
    BYTE drv        /* Physical drive nmuber (0) */
)
{
    if (drv) return STA_NOINIT;        /* Supports only single drive */
    return Stat;
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
  if (drv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  if( hostif_lseek( fd, sector * 512, SEEK_SET ) == -1 )
    return RES_ERROR;
  if( hostif_read( fd, buff, count * 512 ) != count * 512 )
    return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE drv,            /* Physical drive nmuber (0) */
    const BYTE *buff,    /* Pointer to the data to be written */
    DWORD sector,        /* Start sector number (LBA) */
    BYTE count            /* Sector count (1..255) */
)
{
  if (drv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  if( hostif_lseek( fd, sector * 512, SEEK_SET ) == -1 )
    return RES_ERROR;
  if( hostif_write( fd, buff, count * 512 ) != count * 512 )
    return RES_ERROR;
  return RES_OK;
}

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
    BYTE *ptr = buff;

    if (drv) return RES_PARERR;
    res = RES_ERROR;

    if (ctrl == CTRL_POWER) {
        switch (*ptr) {
        case 0:        /* Sub control code == 0 (POWER_OFF) */
            if (chk_power())
                power_off();        /* Power off */
            res = RES_OK;
            break;
        case 1:        /* Sub control code == 1 (POWER_ON) */
            power_on();                /* Power on */
            res = RES_OK;
            break;
        case 2:        /* Sub control code == 2 (POWER_GET) */
            *(ptr+1) = (BYTE)chk_power();
            res = RES_OK;
            break;
        default :
            res = RES_PARERR;
        }
    }
    else {
        if (Stat & STA_NOINIT) return RES_NOTRDY;

        switch (ctrl) {
        case GET_SECTOR_COUNT :    /* Get number of sectors on the disk (DWORD) */
            *(DWORD*)buff = imgsize / 512;
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE :    /* Get sectors on the disk (WORD) */
            *(WORD*)buff = 512;
            res = RES_OK;
            break;

        case CTRL_SYNC :    /* Make sure that data has been written */
            res = RES_OK;
            break;

        default:
            printf( "[SDSIM] unhandled IOCTL %d\n", ctrl );
            res = RES_PARERR;
        }
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
#endif // #if defined( BUILD_MMCFS ) && defined( ELUA_SIMULATOR )

