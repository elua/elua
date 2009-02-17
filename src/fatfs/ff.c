/*----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.06                     (C)ChaN, 2008
/-----------------------------------------------------------------------------/
/ The FatFs module is an experimenal project to implement FAT file system to
/ cheap microcontrollers. This is a free software and is opened for education,
/ research and development under license policy of following trems.
/
/  Copyright (C) 2008, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is no warranty.
/ * You can use, modify and/or redistribute it for personal, non-profit or
/   commercial use without restriction under your responsibility.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
/ Feb 26,'06 R0.00  Prototype.
/
/ Apr 29,'06 R0.01  First stable version.
/
/ Jun 01,'06 R0.02  Added FAT12 support.
/                   Removed unbuffered mode.
/                   Fixed a problem on small (<32M) patition.
/ Jun 10,'06 R0.02a Added a configuration option (_FS_MINIMUM).
/
/ Sep 22,'06 R0.03  Added f_rename().
/                   Changed option _FS_MINIMUM to _FS_MINIMIZE.
/ Dec 11,'06 R0.03a Improved cluster scan algolithm to write files fast.
/                   Fixed f_mkdir() creates incorrect directory on FAT32.
/
/ Feb 04,'07 R0.04  Supported multiple drive system.
/                   Changed some interfaces for multiple drive system.
/                   Changed f_mountdrv() to f_mount().
/                   Added f_mkfs().
/ Apr 01,'07 R0.04a Supported multiple partitions on a plysical drive.
/                   Added a capability of extending file size to f_lseek().
/                   Added minimization level 3.
/                   Fixed an endian sensitive code in f_mkfs().
/ May 05,'07 R0.04b Added a configuration option _USE_NTFLAG.
/                   Added FSInfo support.
/                   Fixed DBCS name can result FR_INVALID_NAME.
/                   Fixed short seek (<= csize) collapses the file object.
/
/ Aug 25,'07 R0.05  Changed arguments of f_read(), f_write() and f_mkfs().
/                   Fixed f_mkfs() on FAT32 creates incorrect FSInfo.
/                   Fixed f_mkdir() on FAT32 creates incorrect directory.
/ Feb 03,'08 R0.05a Added f_truncate() and f_utime().
/                   Fixed off by one error at FAT sub-type determination.
/                   Fixed btr in f_read() can be mistruncated.
/                   Fixed cached sector is not flushed when create and close
/                   without write.
/
/ Apr 01,'08 R0.06  Added fputc(), fputs(), fprintf() and fgets().
/                   Improved performance of f_lseek() on moving to the same
/                   or following cluster.
/---------------------------------------------------------------------------*/

#include <string.h>
#include "ff.h"			/* FatFs declarations */
#include "diskio.h"		/* Include file for user provided disk functions */


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static
FATFS *FatFs[_DRIVES];	/* Pointer to the file system objects (logical drives) */
static
WORD fsid;				/* File system mount ID */



/*-----------------------------------------------------------------------*/
/* Change window offset                                                  */
/*-----------------------------------------------------------------------*/

static
BOOL move_window (	/* TRUE: successful, FALSE: failed */
	FATFS *fs,		/* File system object */
	DWORD sector	/* Sector number to make apperance in the fs->win[] */
)					/* Move to zero only writes back dirty window */
{
	DWORD wsect;


	wsect = fs->winsect;
	if (wsect != sector) {	/* Changed current window */
#if !_FS_READONLY
		BYTE n;
		if (fs->winflag) {	/* Write back dirty window if needed */
			if (disk_write(fs->drive, fs->win, wsect, 1) != RES_OK)
				return FALSE;
			fs->winflag = 0;
			if (wsect < (fs->fatbase + fs->sects_fat)) {	/* In FAT area */
				for (n = fs->n_fats; n >= 2; n--) {	/* Refrect the change to FAT copy */
					wsect += fs->sects_fat;
					disk_write(fs->drive, fs->win, wsect, 1);
				}
			}
		}
#endif
		if (sector) {
			if (disk_read(fs->drive, fs->win, sector, 1) != RES_OK)
				return FALSE;
			fs->winsect = sector;
		}
	}
	return TRUE;
}




/*-----------------------------------------------------------------------*/
/* Clean-up cached data                                                  */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
FRESULT sync (	/* FR_OK: successful, FR_RW_ERROR: failed */
	FATFS *fs	/* File system object */
)
{
	fs->winflag = 1;
	if (!move_window(fs, 0)) return FR_RW_ERROR;
#if _USE_FSINFO
	/* Update FSInfo sector if needed */
	if (fs->fs_type == FS_FAT32 && fs->fsi_flag) {
		fs->winsect = 0;
		memset(fs->win, 0, 512);
		ST_WORD(&fs->win[BS_55AA], 0xAA55);
		ST_DWORD(&fs->win[FSI_LeadSig], 0x41615252);
		ST_DWORD(&fs->win[FSI_StrucSig], 0x61417272);
		ST_DWORD(&fs->win[FSI_Free_Count], fs->free_clust);
		ST_DWORD(&fs->win[FSI_Nxt_Free], fs->last_clust);
		disk_write(fs->drive, fs->win, fs->fsi_sector, 1);
		fs->fsi_flag = 0;
	}
#endif
	/* Make sure that no pending write process in the physical drive */
	if (disk_ioctl(fs->drive, CTRL_SYNC, NULL) != RES_OK)
		return FR_RW_ERROR;
	return FR_OK;
}
#endif




/*-----------------------------------------------------------------------*/
/* Get a cluster status                                                  */
/*-----------------------------------------------------------------------*/

static
DWORD get_cluster (	/* 0,>=2: successful, 1: failed */
	FATFS *fs,		/* File system object */
	DWORD clust		/* Cluster# to get the link information */
)
{
	WORD wc, bc;
	DWORD fatsect;


	if (clust >= 2 && clust < fs->max_clust) {		/* Is it a valid cluster#? */
		fatsect = fs->fatbase;
		switch (fs->fs_type) {
		case FS_FAT12 :
			bc = (WORD)clust * 3 / 2;
			if (!move_window(fs, fatsect + (bc / SS(fs)))) break;
			wc = fs->win[bc & (SS(fs) - 1)]; bc++;
			if (!move_window(fs, fatsect + (bc / SS(fs)))) break;
			wc |= (WORD)fs->win[bc & (SS(fs) - 1)] << 8;
			return (clust & 1) ? (wc >> 4) : (wc & 0xFFF);

		case FS_FAT16 :
			if (!move_window(fs, fatsect + (clust / (SS(fs) / 2)))) break;
			return LD_WORD(&fs->win[((WORD)clust * 2) & (SS(fs) - 1)]);

		case FS_FAT32 :
			if (!move_window(fs, fatsect + (clust / (SS(fs) / 4)))) break;
			return LD_DWORD(&fs->win[((WORD)clust * 4) & (SS(fs) - 1)]) & 0x0FFFFFFF;
		}
	}

	return 1;	/* Out of cluster range, or an error occured */
}




/*-----------------------------------------------------------------------*/
/* Change a cluster status                                               */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
BOOL put_cluster (	/* TRUE: successful, FALSE: failed */
	FATFS *fs,		/* File system object */
	DWORD clust,	/* Cluster# to change (must be 2 to fs->max_clust-1) */
	DWORD val		/* New value to mark the cluster */
)
{
	WORD bc;
	BYTE *p;
	DWORD fatsect;


	fatsect = fs->fatbase;
	switch (fs->fs_type) {
	case FS_FAT12 :
		bc = (WORD)clust * 3 / 2;
		if (!move_window(fs, fatsect + (bc / SS(fs)))) return FALSE;
		p = &fs->win[bc & (SS(fs) - 1)];
		*p = (clust & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;
		bc++;
		fs->winflag = 1;
		if (!move_window(fs, fatsect + (bc / SS(fs)))) return FALSE;
		p = &fs->win[bc & (SS(fs) - 1)];
		*p = (clust & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));
		break;

	case FS_FAT16 :
		if (!move_window(fs, fatsect + (clust / (SS(fs) / 2)))) return FALSE;
		ST_WORD(&fs->win[((WORD)clust * 2) & (SS(fs) - 1)], (WORD)val);
		break;

	case FS_FAT32 :
		if (!move_window(fs, fatsect + (clust / (SS(fs) / 4)))) return FALSE;
		ST_DWORD(&fs->win[((WORD)clust * 4) & (SS(fs) - 1)], val);
		break;

	default :
		return FALSE;
	}
	fs->winflag = 1;
	return TRUE;
}
#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Remove a cluster chain                                                */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
BOOL remove_chain (	/* TRUE: successful, FALSE: failed */
	FATFS *fs,		/* File system object */
	DWORD clust		/* Cluster# to remove chain from */
)
{
	DWORD nxt;


	while (clust >= 2 && clust < fs->max_clust) {
		nxt = get_cluster(fs, clust);
		if (nxt == 1) return FALSE;
		if (!put_cluster(fs, clust, 0)) return FALSE;
		if (fs->free_clust != 0xFFFFFFFF) {
			fs->free_clust++;
#if _USE_FSINFO
			fs->fsi_flag = 1;
#endif
		}
		clust = nxt;
	}
	return TRUE;
}
#endif




/*-----------------------------------------------------------------------*/
/* Stretch or create a cluster chain                                     */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
DWORD create_chain (	/* 0: No free cluster, 1: Error, >=2: New cluster number */
	FATFS *fs,			/* File system object */
	DWORD clust			/* Cluster# to stretch, 0 means create new */
)
{
	DWORD cstat, ncl, scl, mcl = fs->max_clust;


	if (clust == 0) {		/* Create new chain */
		scl = fs->last_clust;			/* Get suggested start point */
		if (scl == 0 || scl >= mcl) scl = 1;
	}
	else {					/* Stretch existing chain */
		cstat = get_cluster(fs, clust);	/* Check the cluster status */
		if (cstat < 2) return 1;		/* It is an invalid cluster */
		if (cstat < mcl) return cstat;	/* It is already followed by next cluster */
		scl = clust;
	}

	ncl = scl;				/* Start cluster */
	for (;;) {
		ncl++;							/* Next cluster */
		if (ncl >= mcl) {				/* Wrap around */
			ncl = 2;
			if (ncl > scl) return 0;	/* No free custer */
		}
		cstat = get_cluster(fs, ncl);	/* Get the cluster status */
		if (cstat == 0) break;			/* Found a free cluster */
		if (cstat == 1) return 1;		/* Any error occured */
		if (ncl == scl) return 0;		/* No free custer */
	}

	if (!put_cluster(fs, ncl, 0x0FFFFFFF)) return 1;			/* Mark the new cluster "in use" */
	if (clust != 0 && !put_cluster(fs, clust, ncl)) return 1;	/* Link it to previous one if needed */

	fs->last_clust = ncl;				/* Update fsinfo */
	if (fs->free_clust != 0xFFFFFFFF) {
		fs->free_clust--;
#if _USE_FSINFO
		fs->fsi_flag = 1;
#endif
	}

	return ncl;		/* Return new cluster number */
}
#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/

static
DWORD clust2sect (	/* !=0: sector number, 0: failed - invalid cluster# */
	FATFS *fs,		/* File system object */
	DWORD clust		/* Cluster# to be converted */
)
{
	clust -= 2;
	if (clust >= (fs->max_clust - 2)) return 0;		/* Invalid cluster# */
	return clust * fs->csize + fs->database;
}




/*-----------------------------------------------------------------------*/
/* Move directory pointer to next                                        */
/*-----------------------------------------------------------------------*/

static
BOOL next_dir_entry (	/* TRUE: successful, FALSE: could not move next */
	DIR *dj				/* Pointer to directory object */
)
{
	DWORD clust;
	WORD idx;


	idx = dj->index + 1;
	if ((idx & ((SS(dj->fs) - 1) / 32)) == 0) {		/* Table sector changed? */
		dj->sect++;				/* Next sector */
		if (dj->clust == 0) {	/* In static table */
			if (idx >= dj->fs->n_rootdir) return FALSE;	/* Reached to end of table */
		} else {					/* In dynamic table */
			if (((idx / (SS(dj->fs) / 32)) & (dj->fs->csize - 1)) == 0) {	/* Cluster changed? */
				clust = get_cluster(dj->fs, dj->clust);			/* Get next cluster */
				if (clust < 2 || clust >= dj->fs->max_clust)	/* Reached to end of table */
					return FALSE;
				dj->clust = clust;				/* Initialize for new cluster */
				dj->sect = clust2sect(dj->fs, clust);
			}
		}
	}
	dj->index = idx;	/* Lower several bits of dj->index indicates offset in dj->sect */
	return TRUE;
}




/*-----------------------------------------------------------------------*/
/* Get file status from directory entry                                  */
/*-----------------------------------------------------------------------*/

#if _FS_MINIMIZE <= 1
static
void get_fileinfo (	/* No return code */
	FILINFO *finfo, /* Ptr to store the file information */
	const BYTE *dir	/* Ptr to the directory entry */
)
{
	BYTE n, c, a;
	char *p;


	p = &finfo->fname[0];
	a = _USE_NTFLAG ? dir[DIR_NTres] : 0;		/* NT flag */
	for (n = 0; n < 8; n++) {	/* Convert file name (body) */
		c = dir[n];
		if (c == ' ') break;
		if (c == 0x05) c = 0xE5;
		if (a & 0x08 && c >= 'A' && c <= 'Z') c += 0x20;
		*p++ = c;
	}
	if (dir[8] != ' ') {		/* Convert file name (extension) */
		*p++ = '.';
		for (n = 8; n < 11; n++) {
			c = dir[n];
			if (c == ' ') break;
			if (a & 0x10 && c >= 'A' && c <= 'Z') c += 0x20;
			*p++ = c;
		}
	}
	*p = '\0';

	finfo->fattrib = dir[DIR_Attr];					/* Attribute */
	finfo->fsize = LD_DWORD(&dir[DIR_FileSize]);	/* Size */
	finfo->fdate = LD_WORD(&dir[DIR_WrtDate]);		/* Date */
	finfo->ftime = LD_WORD(&dir[DIR_WrtTime]);		/* Time */
}
#endif /* _FS_MINIMIZE <= 1 */




/*-----------------------------------------------------------------------*/
/* Pick a paragraph and create the name in format of directory entry     */
/*-----------------------------------------------------------------------*/

static
char make_dirfile (		/* 1: error - detected an invalid format, '\0'or'/': next character */
	const char **path,	/* Pointer to the file path pointer */
	char *dirname		/* Pointer to directory name buffer {Name(8), Ext(3), NT flag(1)} */
)
{
	BYTE n, t, c, a, b;


	memset(dirname, ' ', 8+3);	/* Fill buffer with spaces */
	a = 0; b = 0x18;	/* NT flag */
	n = 0; t = 8;
	for (;;) {
		c = *(*path)++;
		if (c == '\0' || c == '/') {		/* Reached to end of str or directory separator */
			if (n == 0) break;
			dirname[11] = _USE_NTFLAG ? (a & b) : 0;
			return c;
		}
		if (c <= ' ' || c == 0x7F) break;		/* Reject invisible chars */
		if (c == '.') {
			if (!(a & 1) && n >= 1 && n <= 8) {	/* Enter extension part */
				n = 8; t = 11; continue;
			}
			break;
		}
		if (_USE_SJIS &&
			((c >= 0x81 && c <= 0x9F) ||	/* Accept S-JIS code */
		    (c >= 0xE0 && c <= 0xFC))) {
			if (n == 0 && c == 0xE5)		/* Change heading \xE5 to \x05 */
				c = 0x05;
			a ^= 0x01; goto md_l2;
		}
		if (c == '"') break;				/* Reject " */
		if (c <= ')') goto md_l1;			/* Accept ! # $ % & ' ( ) */
		if (c <= ',') break;				/* Reject * + , */
		if (c <= '9') goto md_l1;			/* Accept - 0-9 */
		if (c <= '?') break;				/* Reject : ; < = > ? */
		if (!(a & 1)) {	/* These checks are not applied to S-JIS 2nd byte */
			if (c == '|') break;			/* Reject | */
			if (c >= '[' && c <= ']') break;/* Reject [ \ ] */
			if (_USE_NTFLAG && c >= 'A' && c <= 'Z')
				(t == 8) ? (b &= 0xF7) : (b &= 0xEF);
			if (c >= 'a' && c <= 'z') {		/* Convert to upper case */
				c -= 0x20;
				if (_USE_NTFLAG) (t == 8) ? (a |= 0x08) : (a |= 0x10);
			}
		}
	md_l1:
		a &= 0xFE;
	md_l2:
		if (n >= t) break;
		dirname[n++] = c;
	}
	return 1;
}




/*-----------------------------------------------------------------------*/
/* Trace a file path                                                     */
/*-----------------------------------------------------------------------*/

static
FRESULT trace_path (	/* FR_OK(0): successful, !=0: error code */
	DIR *dj,			/* Pointer to directory object to return last directory */
	char *fn,			/* Pointer to last segment name to return {file(8),ext(3),attr(1)} */
	const char *path,	/* Full-path string to trace a file or directory */
	BYTE **dir			/* Pointer to pointer to found entry to retutn */
)
{
	DWORD clust;
	char ds;
	BYTE *dptr = NULL;
	FATFS *fs = dj->fs;


	/* Initialize directory object */
	clust = fs->dirbase;
	if (fs->fs_type == FS_FAT32) {
		dj->clust = dj->sclust = clust;
		dj->sect = clust2sect(fs, clust);
	} else {
		dj->clust = dj->sclust = 0;
		dj->sect = clust;
	}
	dj->index = 0;

	if (*path == '\0') {					/* Null path means the root directory */
		*dir = NULL; return FR_OK;
	}

	for (;;) {
		ds = make_dirfile(&path, fn);			/* Get a paragraph into fn[] */
		if (ds == 1) return FR_INVALID_NAME;
		for (;;) {
			if (!move_window(fs, dj->sect)) return FR_RW_ERROR;
			dptr = &fs->win[(dj->index & ((SS(fs) - 1) / 32)) * 32];	/* Pointer to the directory entry */
			if (dptr[DIR_Name] == 0)						/* Has it reached to end of dir? */
				return !ds ? FR_NO_FILE : FR_NO_PATH;
			if (dptr[DIR_Name] != 0xE5						/* Matched? */
				&& !(dptr[DIR_Attr] & AM_VOL)
				&& !memcmp(&dptr[DIR_Name], fn, 8+3) ) break;
			if (!next_dir_entry(dj))						/* Next directory pointer */
				return !ds ? FR_NO_FILE : FR_NO_PATH;
		}
		if (!ds) { *dir = dptr; return FR_OK; }				/* Matched with end of path */
		if (!(dptr[DIR_Attr] & AM_DIR)) return FR_NO_PATH;	/* Cannot trace because it is a file */
		clust = ((DWORD)LD_WORD(&dptr[DIR_FstClusHI]) << 16) | LD_WORD(&dptr[DIR_FstClusLO]); /* Get cluster# of the directory */
		dj->clust = dj->sclust = clust;				/* Restart scanning at the new directory */
		dj->sect = clust2sect(fs, clust);
		dj->index = 2;
	}
}




/*-----------------------------------------------------------------------*/
/* Reserve a directory entry                                             */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
FRESULT reserve_direntry (	/* FR_OK: successful, FR_DENIED: no free entry, FR_RW_ERROR: a disk error occured */
	DIR *dj,				/* Target directory to create new entry */
	BYTE **dir				/* Pointer to pointer to created entry to retutn */
)
{
	DWORD clust, sector;
	BYTE c, n, *dptr;
	FATFS *fs = dj->fs;


	/* Re-initialize directory object */
	clust = dj->sclust;
	if (clust != 0) {	/* Dyanmic directory table */
		dj->clust = clust;
		dj->sect = clust2sect(fs, clust);
	} else {			/* Static directory table */
		dj->sect = fs->dirbase;
	}
	dj->index = 0;

	do {
		if (!move_window(fs, dj->sect)) return FR_RW_ERROR;
		dptr = &fs->win[(dj->index & ((SS(dj->fs) - 1) / 32)) * 32];	/* Pointer to the directory entry */
		c = dptr[DIR_Name];
		if (c == 0 || c == 0xE5) {		/* Found an empty entry */
			*dir = dptr; return FR_OK;
		}
	} while (next_dir_entry(dj));		/* Next directory pointer */
	/* Reached to end of the directory table */

	/* Abort when it is a static table or could not stretch dynamic table */
	if (clust == 0 || !(clust = create_chain(fs, dj->clust))) return FR_DENIED;
	if (clust == 1 || !move_window(fs, 0)) return FR_RW_ERROR;

	/* Cleanup the expanded table */
	fs->winsect = sector = clust2sect(fs, clust);
	memset(fs->win, 0, SS(fs));
	for (n = fs->csize; n; n--) {
		if (disk_write(fs->drive, fs->win, sector, 1) != RES_OK)
			return FR_RW_ERROR;
		sector++;
	}
	fs->winflag = 1;
	*dir = fs->win;

	return FR_OK;
}
#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Load boot record and check if it is an FAT boot record                */
/*-----------------------------------------------------------------------*/

static
BYTE check_fs (	/* 0:The FAT boot record, 1:Valid boot record but not an FAT, 2:Not a boot record or error */
	FATFS *fs,	/* File system object */
	DWORD sect	/* Sector# (lba) to check if it is an FAT boot record or not */
)
{
	if (disk_read(fs->drive, fs->win, sect, 1) != RES_OK)	/* Load boot record */
		return 2;
	if (LD_WORD(&fs->win[BS_55AA]) != 0xAA55)				/* Check record signature (always placed at offset 510 even if the sector size is >512) */
		return 2;

	if (!memcmp(&fs->win[BS_FilSysType], "FAT", 3))			/* Check FAT signature */
		return 0;
	if (!memcmp(&fs->win[BS_FilSysType32], "FAT32", 5) && !(fs->win[BPB_ExtFlags] & 0x80))
		return 0;

	return 1;
}




/*-----------------------------------------------------------------------*/
/* Make sure that the file system is valid                               */
/*-----------------------------------------------------------------------*/

static
FRESULT auto_mount (	/* FR_OK(0): successful, !=0: any error occured */
	const char **path,	/* Pointer to pointer to the path name (drive number) */
	FATFS **rfs,		/* Pointer to pointer to the found file system object */
	BYTE chk_wp			/* !=0: Check media write protection for write access */
)
{
	BYTE drv, fmt, *tbl;
	DSTATUS stat;
	DWORD bootsect, fatsize, totalsect, maxclust;
	const char *p = *path;
	FATFS *fs;


	/* Get drive number from the path name */
	while (*p == ' ') p++;		/* Strip leading spaces */
	drv = p[0] - '0';			/* Is there a drive number? */
	if (drv <= 9 && p[1] == ':')
		p += 2;				/* Found a drive number, get and strip it */
	else
		drv = 0;			/* No drive number is given, use drive number 0 as default */
	if (*p == '/') p++;		/* Strip heading slash */
	*path = p;				/* Return pointer to the path name */

	/* Check if the drive number is valid or not */
	if (drv >= _DRIVES) return FR_INVALID_DRIVE;	/* Is the drive number valid? */
	*rfs = fs = FatFs[drv];					/* Returen pointer to the corresponding file system object */
	if (!fs) return FR_NOT_ENABLED;			/* Is the file system object registered? */

	if (fs->fs_type) {						/* If the logical drive has been mounted */
		stat = disk_status(fs->drive);
		if (!(stat & STA_NOINIT)) {			/* and physical drive is kept initialized (has not been changed), */
#if !_FS_READONLY
			if (chk_wp && (stat & STA_PROTECT))	/* Check write protection if needed */
				return FR_WRITE_PROTECTED;
#endif
			return FR_OK;					/* The file system object is valid */
		}
	}

	/* The logical drive must be re-mounted. Following code attempts to mount the logical drive */

	memset(fs, 0, sizeof(FATFS));		/* Clean-up the file system object */
	fs->drive = LD2PD(drv);				/* Bind the logical drive and a physical drive */
	stat = disk_initialize(fs->drive);	/* Initialize low level disk I/O layer */
	if (stat & STA_NOINIT)				/* Check if the drive is ready */
		return FR_NOT_READY;
#if S_MAX_SIZ > 512						/* Get disk sector size if needed */
	if (disk_ioctl(drv, GET_SECTOR_SIZE, &SS(fs)) != RES_OK || SS(fs) > S_MAX_SIZ)
		return FR_NO_FILESYSTEM;
#endif
#if !_FS_READONLY
	if (chk_wp && (stat & STA_PROTECT))	/* Check write protection if needed */
		return FR_WRITE_PROTECTED;
#endif
	/* Search FAT partition on the drive */
	fmt = check_fs(fs, bootsect = 0);	/* Check sector 0 as an SFD format */
	if (fmt == 1) {						/* Not an FAT boot record, it may be patitioned */
		/* Check a partition listed in top of the partition table */
		tbl = &fs->win[MBR_Table + LD2PT(drv) * 16];	/* Partition table */
		if (tbl[4]) {									/* Is the partition existing? */
			bootsect = LD_DWORD(&tbl[8]);				/* Partition offset in LBA */
			fmt = check_fs(fs, bootsect);				/* Check the partition */
		}
	}
	if (fmt || LD_WORD(&fs->win[BPB_BytsPerSec]) != SS(fs))	/* No valid FAT patition is found */
		return FR_NO_FILESYSTEM;

	/* Initialize the file system object */
	fatsize = LD_WORD(&fs->win[BPB_FATSz16]);			/* Number of sectors per FAT */
	if (!fatsize) fatsize = LD_DWORD(&fs->win[BPB_FATSz32]);
	fs->sects_fat = fatsize;
	fs->n_fats = fs->win[BPB_NumFATs];					/* Number of FAT copies */
	fatsize *= fs->n_fats;								/* (Number of sectors in FAT area) */
	fs->fatbase = bootsect + LD_WORD(&fs->win[BPB_RsvdSecCnt]); /* FAT start sector (lba) */
	fs->csize = fs->win[BPB_SecPerClus];				/* Number of sectors per cluster */
	fs->n_rootdir = LD_WORD(&fs->win[BPB_RootEntCnt]);	/* Nmuber of root directory entries */
	totalsect = LD_WORD(&fs->win[BPB_TotSec16]);		/* Number of sectors on the file system */
	if (!totalsect) totalsect = LD_DWORD(&fs->win[BPB_TotSec32]);
	fs->max_clust = maxclust = (totalsect				/* max_clust = Last cluster# + 1 */
		- LD_WORD(&fs->win[BPB_RsvdSecCnt]) - fatsize - fs->n_rootdir / (SS(fs)/32)
		) / fs->csize + 2;

	fmt = FS_FAT12;										/* Determine the FAT sub type */
	if (maxclust >= 0xFF7) fmt = FS_FAT16;
	if (maxclust >= 0xFFF7) fmt = FS_FAT32;

	if (fmt == FS_FAT32)
		fs->dirbase = LD_DWORD(&fs->win[BPB_RootClus]);	/* Root directory start cluster */
	else
		fs->dirbase = fs->fatbase + fatsize;			/* Root directory start sector (lba) */
	fs->database = fs->fatbase + fatsize + fs->n_rootdir / (SS(fs)/32);	/* Data start sector (lba) */

#if !_FS_READONLY
	/* Initialize allocation information */
	fs->free_clust = 0xFFFFFFFF;
#if _USE_FSINFO
	/* Get fsinfo if needed */
	if (fmt == FS_FAT32) {
		fs->fsi_sector = bootsect + LD_WORD(&fs->win[BPB_FSInfo]);
		if (disk_read(fs->drive, fs->win, fs->fsi_sector, 1) == RES_OK &&
			LD_WORD(&fs->win[BS_55AA]) == 0xAA55 &&
			LD_DWORD(&fs->win[FSI_LeadSig]) == 0x41615252 &&
			LD_DWORD(&fs->win[FSI_StrucSig]) == 0x61417272) {
			fs->last_clust = LD_DWORD(&fs->win[FSI_Nxt_Free]);
			fs->free_clust = LD_DWORD(&fs->win[FSI_Free_Count]);
		}
	}
#endif
#endif

	fs->fs_type = fmt;			/* FAT syb-type */
	fs->id = ++fsid;			/* File system mount ID */
	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Check if the file/dir object is valid or not                          */
/*-----------------------------------------------------------------------*/

static
FRESULT validate (		/* FR_OK(0): The object is valid, !=0: Invalid */
	const FATFS *fs,	/* Pointer to the file system object */
	WORD id				/* Member id of the target object to be checked */
)
{
	if (!fs || !fs->fs_type || fs->id != id)
		return FR_INVALID_OBJECT;
	if (disk_status(fs->drive) & STA_NOINIT)
		return FR_NOT_READY;

	return FR_OK;
}




/*--------------------------------------------------------------------------

   Public Functions

--------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Locical Drive                                         */
/*-----------------------------------------------------------------------*/

FRESULT f_mount (
	BYTE drv,		/* Logical drive number to be mounted/unmounted */
	FATFS *fs		/* Pointer to new file system object (NULL for unmount)*/
)
{
	if (drv >= _DRIVES) return FR_INVALID_DRIVE;

	if (FatFs[drv]) FatFs[drv]->fs_type = 0;	/* Clear old object */

	FatFs[drv] = fs;			/* Register and clear new object */
	if (fs) fs->fs_type = 0;

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_open (
	FIL *fp,			/* Pointer to the blank file object */
	const char *path,	/* Pointer to the file name */
	BYTE mode			/* Access mode and file open mode flags */
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	char fn[8+3+1];


	fp->fs = NULL;		/* Clear file object */
#if !_FS_READONLY
	mode &= (FA_READ|FA_WRITE|FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW);
	res = auto_mount(&path, &dj.fs, (BYTE)(mode & (FA_WRITE|FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW)));
#else
	mode &= FA_READ;
	res = auto_mount(&path, &dj.fs, 0);
#endif
	if (res != FR_OK) return res;
	res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */

#if !_FS_READONLY
	/* Create or Open a file */
	if (mode & (FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW)) {
		DWORD ps, rs;
		if (res != FR_OK) {		/* No file, create new */
			if (res != FR_NO_FILE) return res;
			res = reserve_direntry(&dj, &dir);
			if (res != FR_OK) return res;
			memset(dir, 0, 32);		/* Initialize the new entry with open name */
			memcpy(&dir[DIR_Name], fn, 8+3);
			dir[DIR_NTres] = fn[11];
			mode |= FA_CREATE_ALWAYS;
		}
		else {					/* Any object is already existing */
			if (mode & FA_CREATE_NEW)			/* Cannot create new */
				return FR_EXIST;
			if (!dir || (dir[DIR_Attr] & (AM_RDO|AM_DIR)))	/* Cannot overwrite it (R/O or DIR) */
				return FR_DENIED;
			if (mode & FA_CREATE_ALWAYS) {		/* Resize it to zero if needed */
				rs = ((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);	/* Get start cluster */
				ST_WORD(&dir[DIR_FstClusHI], 0);	/* cluster = 0 */
				ST_WORD(&dir[DIR_FstClusLO], 0);
				ST_DWORD(&dir[DIR_FileSize], 0);	/* size = 0 */
				dj.fs->winflag = 1;
				ps = dj.fs->winsect;			/* Remove the cluster chain */
				if (!remove_chain(dj.fs, rs) || !move_window(dj.fs, ps))
					return FR_RW_ERROR;
				dj.fs->last_clust = rs - 1;		/* Reuse the cluster hole */
			}
		}
		if (mode & FA_CREATE_ALWAYS) {
			dir[DIR_Attr] = 0;					/* Reset attribute */
			ps = get_fattime();
			ST_DWORD(&dir[DIR_CrtTime], ps);	/* Created time */
			dj.fs->winflag = 1;
			mode |= FA__WRITTEN;				/* Set file changed flag */
		}
	}
	/* Open an existing file */
	else {
#endif /* !_FS_READONLY */
		if (res != FR_OK) return res;			/* Trace failed */
		if (!dir || (dir[DIR_Attr] & AM_DIR))	/* It is a directory */
			return FR_NO_FILE;
#if !_FS_READONLY
		if ((mode & FA_WRITE) && (dir[DIR_Attr] & AM_RDO)) /* R/O violation */
			return FR_DENIED;
	}
	fp->dir_sect = dj.fs->winsect;		/* Pointer to the directory entry */
	fp->dir_ptr = dir;
#endif
	fp->flag = mode;					/* File access mode */
	fp->org_clust =						/* File start cluster */
		((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);
	fp->fsize = LD_DWORD(&dir[DIR_FileSize]);	/* File size */
	fp->fptr = 0; fp->csect = 255;		/* File pointer */
	fp->curr_sect = 0;
	fp->fs = dj.fs; fp->id = dj.fs->id;	/* Owner file system object of the file */

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_read (
	FIL *fp, 		/* Pointer to the file object */
	void *buff,		/* Pointer to data buffer */
	UINT btr,		/* Number of bytes to read */
	UINT *br		/* Pointer to number of bytes read */
)
{
	FRESULT res;
	DWORD clust, sect, remain;
	UINT rcnt, cc;
	BYTE *rbuff = buff;


	*br = 0;
	res = validate(fp->fs, fp->id);					/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
	if (!(fp->flag & FA_READ)) return FR_DENIED;	/* Check access mode */
	remain = fp->fsize - fp->fptr;
	if (btr > remain) btr = (UINT)remain;			/* Truncate btr by remaining bytes */

	for ( ;  btr;									/* Repeat until all data transferred */
		rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt) {
		if ((fp->fptr % SS(fp->fs)) == 0) {			/* On the sector boundary? */
			if (fp->csect >= fp->fs->csize) {		/* On the cluster boundary? */
				clust = (fp->fptr == 0) ?			/* On the top of the file? */
					fp->org_clust : get_cluster(fp->fs, fp->curr_clust);
				if (clust < 2 || clust >= fp->fs->max_clust) goto fr_error;
				fp->curr_clust = clust;				/* Update current cluster */
				fp->csect = 0;						/* Reset sector address in the cluster */
			}
			sect = clust2sect(fp->fs, fp->curr_clust) + fp->csect;	/* Get current sector */
			cc = btr / SS(fp->fs);					/* When remaining bytes >= sector size, */
			if (cc) {								/* Read maximum contiguous sectors directly */
				if (fp->csect + cc > fp->fs->csize)	/* Clip at cluster boundary */
					cc = fp->fs->csize - fp->csect;
				if (disk_read(fp->fs->drive, rbuff, sect, (BYTE)cc) != RES_OK)
					goto fr_error;
				fp->csect += (BYTE)cc;				/* Next sector address in the cluster */
				rcnt = SS(fp->fs) * cc;				/* Number of bytes transferred */
				continue;
			}
			if (sect != fp->curr_sect) {			/* Is window offset changed? */
#if !_FS_READONLY
				if (fp->flag & FA__DIRTY) {			/* Write back file I/O buffer if needed */
					if (disk_write(fp->fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
						goto fr_error;
					fp->flag &= (BYTE)~FA__DIRTY;
				}
#endif
				if (disk_read(fp->fs->drive, fp->buffer, sect, 1) != RES_OK)	/* Fill file I/O buffer with file data */
					goto fr_error;
				fp->curr_sect = sect;
			}
			fp->csect++;							/* Next sector address in the cluster */
		}
		rcnt = SS(fp->fs) - (fp->fptr % SS(fp->fs));	/* Get partial sector from file I/O buffer */
		if (rcnt > btr) rcnt = btr;
		memcpy(rbuff, &fp->buffer[fp->fptr % SS(fp->fs)], rcnt);
	}

	return FR_OK;

fr_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}




#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_write (
	FIL *fp,			/* Pointer to the file object */
	const void *buff,	/* Pointer to the data to be written */
	UINT btw,			/* Number of bytes to write */
	UINT *bw			/* Pointer to number of bytes written */
)
{
	FRESULT res;
	DWORD clust, sect;
	UINT wcnt, cc;
	const BYTE *wbuff = buff;


	*bw = 0;
	res = validate(fp->fs, fp->id);					/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
	if (!(fp->flag & FA_WRITE)) return FR_DENIED;	/* Check access mode */
	if (fp->fsize + btw < fp->fsize) return FR_OK;	/* File size cannot reach 4GB */

	for ( ;  btw;									/* Repeat until all data transferred */
		wbuff += wcnt, fp->fptr += wcnt, *bw += wcnt, btw -= wcnt) {
		if ((fp->fptr % SS(fp->fs)) == 0) {			/* On the sector boundary? */
			if (fp->csect >= fp->fs->csize) {		/* On the cluster boundary? */
				if (fp->fptr == 0) {				/* On the top of the file? */
					clust = fp->org_clust;			/* Follow from the origin */
					if (clust == 0)					/* When there is no cluster chain, */
						fp->org_clust = clust = create_chain(fp->fs, 0);	/* Create a new cluster chain */
				} else {							/* Middle or end of the file */
					clust = create_chain(fp->fs, fp->curr_clust);			/* Trace or streach cluster chain */
				}
				if (clust == 0) break;				/* Could not allocate a new cluster (disk full) */
				if (clust == 1 || clust >= fp->fs->max_clust) goto fw_error;
				fp->curr_clust = clust;				/* Update current cluster */
				fp->csect = 0;						/* Reset sector address in the cluster */
			}
			sect = clust2sect(fp->fs, fp->curr_clust) + fp->csect;	/* Get current sector */
			cc = btw / SS(fp->fs);					/* When remaining bytes >= sector size, */
			if (cc) {								/* Write maximum contiguous sectors directly */
				if (fp->csect + cc > fp->fs->csize)	/* Clip at cluster boundary */
					cc = fp->fs->csize - fp->csect;
				if (disk_write(fp->fs->drive, wbuff, sect, (BYTE)cc) != RES_OK)
					goto fw_error;
				fp->csect += (BYTE)cc;				/* Next sector address in the cluster */
				wcnt = SS(fp->fs) * cc;				/* Number of bytes transferred */
				continue;
			}
			if (sect != fp->curr_sect) {			/* Is window offset changed? */
				if (fp->flag & FA__DIRTY) {			/* Write back file I/O buffer if needed */
					if (disk_write(fp->fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
						goto fw_error;
					fp->flag &= (BYTE)~FA__DIRTY;
				}
				if (fp->fptr < fp->fsize &&  		/* Fill file I/O buffer with file data */
					disk_read(fp->fs->drive, fp->buffer, sect, 1) != RES_OK)
						goto fw_error;
				fp->curr_sect = sect;
			}
			fp->csect++;							/* Next sector address in the cluster */
		}
		wcnt = SS(fp->fs) - (fp->fptr % SS(fp->fs));	/* Put partial sector into file I/O buffer */
		if (wcnt > btw) wcnt = btw;
		memcpy(&fp->buffer[fp->fptr % SS(fp->fs)], wbuff, wcnt);
		fp->flag |= FA__DIRTY;
	}

	if (fp->fptr > fp->fsize) fp->fsize = fp->fptr;	/* Update file size if needed */
	fp->flag |= FA__WRITTEN;						/* Set file changed flag */
	return FR_OK;

fw_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}




/*-----------------------------------------------------------------------*/
/* Synchronize the file object                                           */
/*-----------------------------------------------------------------------*/

FRESULT f_sync (
	FIL *fp		/* Pointer to the file object */
)
{
	FRESULT res;
	DWORD tim;
	BYTE *dir;


	res = validate(fp->fs, fp->id);		/* Check validity of the object */
	if (res == FR_OK) {
		if (fp->flag & FA__WRITTEN) {	/* Has the file been written? */
			/* Write back data buffer if needed */
			if (fp->flag & FA__DIRTY) {
				if (disk_write(fp->fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
					return FR_RW_ERROR;
				fp->flag &= (BYTE)~FA__DIRTY;
			}
			/* Update the directory entry */
			if (!move_window(fp->fs, fp->dir_sect))
				return FR_RW_ERROR;
			dir = fp->dir_ptr;
			dir[DIR_Attr] |= AM_ARC;						/* Set archive bit */
			ST_DWORD(&dir[DIR_FileSize], fp->fsize);		/* Update file size */
			ST_WORD(&dir[DIR_FstClusLO], fp->org_clust);	/* Update start cluster */
			ST_WORD(&dir[DIR_FstClusHI], fp->org_clust >> 16);
			tim = get_fattime();					/* Updated time */
			ST_DWORD(&dir[DIR_WrtTime], tim);
			fp->flag &= (BYTE)~FA__WRITTEN;
			res = sync(fp->fs);
		}
	}
	return res;
}

#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_close (
	FIL *fp		/* Pointer to the file object to be closed */
)
{
	FRESULT res;


#if !_FS_READONLY
	res = f_sync(fp);
#else
	res = validate(fp->fs, fp->id);
#endif
	if (res == FR_OK) fp->fs = NULL;
	return res;
}




#if _FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_lseek (
	FIL *fp,		/* Pointer to the file object */
	DWORD ofs		/* File pointer from top of file */
)
{
	FRESULT res;
	DWORD clust, csize, nsect, ifptr;


	res = validate(fp->fs, fp->id);		/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;
	if (ofs > fp->fsize					/* In read-only mode, clip offset with the file size */
#if !_FS_READONLY
		 && !(fp->flag & FA_WRITE)
#endif
		) ofs = fp->fsize;

	ifptr = fp->fptr;
	fp->fptr = 0; fp->csect = 255;
	nsect = 0;
	if (ofs > 0) {
		csize = (DWORD)fp->fs->csize * SS(fp->fs);	/* Cluster size (byte) */
		if (ifptr > 0 &&
			(ofs - 1) / csize >= (ifptr - 1) / csize) {/* When seek to same or following cluster, */
			fp->fptr = (ifptr - 1) & ~(csize - 1);	/* start from the current cluster */
			ofs -= fp->fptr;
			clust = fp->curr_clust;
		} else {									/* When seek to back cluster, */
			clust = fp->org_clust;					/* start from the first cluster */
#if !_FS_READONLY
			if (clust == 0) {						/* If no cluster chain, create a new chain */
				clust = create_chain(fp->fs, 0);
				if (clust == 1) goto fk_error;
				fp->org_clust = clust;
			}
#endif
			fp->curr_clust = clust;
		}
		if (clust != 0) {
			while (ofs > csize) {					/* Cluster following loop */
#if !_FS_READONLY
				if (fp->flag & FA_WRITE) {			/* Check if in write mode or not */
					clust = create_chain(fp->fs, clust);	/* Force streached if in write mode */
					if (clust == 0) {				/* When disk gets full, clip file size */
						ofs = csize; break;
					}
				} else
#endif
					clust = get_cluster(fp->fs, clust);	/* Follow cluster chain if not in write mode */
				if (clust < 2 || clust >= fp->fs->max_clust) goto fk_error;
				fp->curr_clust = clust;
				fp->fptr += csize;
				ofs -= csize;
			}
			fp->fptr += ofs;
			fp->csect = (BYTE)(ofs / SS(fp->fs));	/* Sector offset in the cluster */
			if (ofs & (SS(fp->fs) - 1)) {
				nsect = clust2sect(fp->fs, clust) + fp->csect;	/* Current sector */
				fp->csect++;
			}
		}
	}
	if (nsect && nsect != fp->curr_sect) {
#if !_FS_READONLY
		if (fp->flag & FA__DIRTY) {			/* Write-back dirty buffer if needed */
			if (disk_write(fp->fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
				goto fk_error;
			fp->flag &= (BYTE)~FA__DIRTY;
		}
#endif
		if (disk_read(fp->fs->drive, fp->buffer, nsect, 1) != RES_OK)
			goto fk_error;
		fp->curr_sect = nsect;
	}

#if !_FS_READONLY
	if (fp->fptr > fp->fsize) {			/* Set changed flag if the file was extended */
		fp->fsize = fp->fptr;
		fp->flag |= FA__WRITTEN;
	}
#endif

	return FR_OK;

fk_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}




#if _FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a directroy object                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_opendir (
	DIR *dj,			/* Pointer to directory object to create */
	const char *path	/* Pointer to the directory path */
)
{
	FRESULT res;
	BYTE *dir;
	char fn[8+3+1];


	res = auto_mount(&path, &dj->fs, 0);
	if (res == FR_OK) {
		res = trace_path(dj, fn, path, &dir);	/* Trace the directory path */
		if (res == FR_OK) {						/* Trace completed */
			if (dir) {							/* It is not the root dir */
				if (dir[DIR_Attr] & AM_DIR) {	/* The entry is a directory */
					dj->clust = ((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);
					dj->sect = clust2sect(dj->fs, dj->clust);
					dj->index = 2;
				} else {						/* The entry is not a directory */
					res = FR_NO_FILE;
				}
			}
			dj->id = dj->fs->id;
		}
	}

	return res;
}




/*-----------------------------------------------------------------------*/
/* Read Directory Entry in Sequense                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_readdir (
	DIR *dj,			/* Pointer to the directory object */
	FILINFO *finfo		/* Pointer to file information to return */
)
{
	BYTE *dir, c, res;


	res = validate(dj->fs, dj->id);			/* Check validity of the object */
	if (res != FR_OK) return res;

	finfo->fname[0] = 0;
	while (dj->sect) {
		if (!move_window(dj->fs, dj->sect))
			return FR_RW_ERROR;
		dir = &dj->fs->win[(dj->index & ((SS(dj->fs) - 1) >> 5)) * 32];	/* pointer to the directory entry */
		c = dir[DIR_Name];
		if (c == 0) break;							/* Has it reached to end of dir? */
		if (c != 0xE5 && !(dir[DIR_Attr] & AM_VOL))	/* Is it a valid entry? */
			get_fileinfo(finfo, dir);
		if (!next_dir_entry(dj)) dj->sect = 0;		/* Next entry */
		if (finfo->fname[0]) break;					/* Found valid entry */
	}

	return FR_OK;
}




#if _FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

FRESULT f_stat (
	const char *path,	/* Pointer to the file path */
	FILINFO *finfo		/* Pointer to file information to return */
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	char fn[8+3+1];


	res = auto_mount(&path, &dj.fs, 0);
	if (res == FR_OK) {
		res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
		if (res == FR_OK) {						/* Trace completed */
			if (dir)	/* Found an object */
				get_fileinfo(finfo, dir);
			else		/* It is root dir */
				res = FR_INVALID_NAME;
		}
	}

	return res;
}



#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/

FRESULT f_truncate (
	FIL *fp		/* Pointer to the file object */
)
{
	FRESULT res;
	DWORD ncl;


	res = validate(fp->fs, fp->id);		/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
	if (!(fp->flag & FA_WRITE)) return FR_DENIED;	/* Check access mode */

	if (fp->fsize > fp->fptr) {
		fp->fsize = fp->fptr;	/* Set file size to current R/W point */
		fp->flag |= FA__WRITTEN;
		if (fp->fptr == 0) {	/* When set file size to zero, remove entire cluster chain */
			if (!remove_chain(fp->fs, fp->org_clust)) goto ft_error;
			fp->org_clust = 0;
		} else {				/* When truncate a part of the file, remove remaining clusters */
			ncl = get_cluster(fp->fs, fp->curr_clust);
			if (ncl < 2) goto ft_error;
			if (ncl < fp->fs->max_clust) {
				if (!put_cluster(fp->fs, fp->curr_clust, 0x0FFFFFFF)) goto ft_error;
				if (!remove_chain(fp->fs, ncl)) goto ft_error;
			}
		}
	}

	return FR_OK;

ft_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}




/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

FRESULT f_getfree (
	const char *drv,	/* Pointer to the logical drive number (root dir) */
	DWORD *nclust,		/* Pointer to the variable to return number of free clusters */
	FATFS **fatfs		/* Pointer to pointer to corresponding file system object to return */
)
{
	FRESULT res;
	DWORD n, clust, sect;
	BYTE fat, f, *p;


	/* Get drive number */
	res = auto_mount(&drv, fatfs, 0);
	if (res != FR_OK) return res;

	/* If number of free cluster is valid, return it without cluster scan. */
	if ((*fatfs)->free_clust <= (*fatfs)->max_clust - 2) {
		*nclust = (*fatfs)->free_clust;
		return FR_OK;
	}

	/* Get number of free clusters */
	fat = (*fatfs)->fs_type;
	n = 0;
	if (fat == FS_FAT12) {
		clust = 2;
		do {
			if ((WORD)get_cluster(*fatfs, clust) == 0) n++;
		} while (++clust < (*fatfs)->max_clust);
	} else {
		clust = (*fatfs)->max_clust;
		sect = (*fatfs)->fatbase;
		f = 0; p = 0;
		do {
			if (!f) {
				if (!move_window(*fatfs, sect++)) return FR_RW_ERROR;
				p = (*fatfs)->win;
			}
			if (fat == FS_FAT16) {
				if (LD_WORD(p) == 0) n++;
				p += 2; f += 1;
			} else {
				if (LD_DWORD(p) == 0) n++;
				p += 4; f += 2;
			}
		} while (--clust);
	}
	(*fatfs)->free_clust = n;
#if _USE_FSINFO
	if (fat == FS_FAT32) (*fatfs)->fsi_flag = 1;
#endif

	*nclust = n;
	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Delete a File or Directory                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_unlink (
	const char *path		/* Pointer to the file or directory path */
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir, *sdir;
	DWORD dclust, dsect;
	char fn[8+3+1];


	res = auto_mount(&path, &dj.fs, 1);
	if (res != FR_OK) return res;
	res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
	if (res != FR_OK) return res;			/* Trace failed */
	if (!dir) return FR_INVALID_NAME;		/* It is the root directory */
	if (dir[DIR_Attr] & AM_RDO) return FR_DENIED;	/* It is a R/O object */
	dsect = dj.fs->winsect;
	dclust = ((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);

	if (dir[DIR_Attr] & AM_DIR) {			/* It is a sub-directory */
		dj.clust = dclust;					/* Check if the sub-dir is empty or not */
		dj.sect = clust2sect(dj.fs, dclust);
		dj.index = 2;
		do {
			if (!move_window(dj.fs, dj.sect)) return FR_RW_ERROR;
			sdir = &dj.fs->win[(dj.index & ((SS(dj.fs) - 1) >> 5)) * 32];
			if (sdir[DIR_Name] == 0) break;
			if (sdir[DIR_Name] != 0xE5 && !(sdir[DIR_Attr] & AM_VOL))
				return FR_DENIED;	/* The directory is not empty */
		} while (next_dir_entry(&dj));
	}

	if (!move_window(dj.fs, dsect)) return FR_RW_ERROR;	/* Mark the directory entry 'deleted' */
	dir[DIR_Name] = 0xE5;
	dj.fs->winflag = 1;
	if (!remove_chain(dj.fs, dclust)) return FR_RW_ERROR;	/* Remove the cluster chain */

	return sync(dj.fs);
}




/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/

FRESULT f_mkdir (
	const char *path		/* Pointer to the directory path */
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir, *fw, n;
	char fn[8+3+1];
	DWORD sect, dsect, dclust, pclust, tim;


	res = auto_mount(&path, &dj.fs, 1);
	if (res != FR_OK) return res;
	res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
	if (res == FR_OK) return FR_EXIST;		/* Any file or directory is already existing */
	if (res != FR_NO_FILE) return res;

	res = reserve_direntry(&dj, &dir); 		/* Reserve a directory entry */
	if (res != FR_OK) return res;
	sect = dj.fs->winsect;
	dclust = create_chain(dj.fs, 0);		/* Allocate a cluster for new directory table */
	if (dclust == 1) return FR_RW_ERROR;
	dsect = clust2sect(dj.fs, dclust);
	if (!dsect) return FR_DENIED;
	if (!move_window(dj.fs, dsect)) return FR_RW_ERROR;

	fw = dj.fs->win;
	memset(fw, 0, SS(dj.fs));				/* Clear the new directory table */
	for (n = 1; n < dj.fs->csize; n++) {
		if (disk_write(dj.fs->drive, fw, ++dsect, 1) != RES_OK)
			return FR_RW_ERROR;
	}
	memset(&fw[DIR_Name], ' ', 8+3);		/* Create "." entry */
	fw[DIR_Name] = '.';
	fw[DIR_Attr] = AM_DIR;
	tim = get_fattime();
	ST_DWORD(&fw[DIR_WrtTime], tim);
	memcpy(&fw[32], &fw[0], 32); fw[33] = '.';	/* Create ".." entry */
	ST_WORD(&fw[   DIR_FstClusLO], dclust);
	ST_WORD(&fw[   DIR_FstClusHI], dclust >> 16);
	pclust = dj.sclust;
	if (dj.fs->fs_type == FS_FAT32 && pclust == dj.fs->dirbase) pclust = 0;
	ST_WORD(&fw[32+DIR_FstClusLO], pclust);
	ST_WORD(&fw[32+DIR_FstClusHI], pclust >> 16);
	dj.fs->winflag = 1;

	if (!move_window(dj.fs, sect)) return FR_RW_ERROR;
	memset(&dir[0], 0, 32);						/* Initialize the new entry */
	memcpy(&dir[DIR_Name], fn, 8+3);			/* Name */
	dir[DIR_NTres] = fn[11];
	dir[DIR_Attr] = AM_DIR;						/* Attribute */
	ST_DWORD(&dir[DIR_WrtTime], tim);			/* Crated time */
	ST_WORD(&dir[DIR_FstClusLO], dclust);		/* Table start cluster */
	ST_WORD(&dir[DIR_FstClusHI], dclust >> 16);

	return sync(dj.fs);
}




/*-----------------------------------------------------------------------*/
/* Change File Attribute                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_chmod (
	const char *path,	/* Pointer to the file path */
	BYTE value,			/* Attribute bits */
	BYTE mask			/* Attribute mask to change */
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	char fn[8+3+1];


	res = auto_mount(&path, &dj.fs, 1);
	if (res == FR_OK) {
		res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
		if (res == FR_OK) {				/* Trace completed */
			if (!dir) {
				res = FR_INVALID_NAME;	/* Root directory */
			} else {
				mask &= AM_RDO|AM_HID|AM_SYS|AM_ARC;	/* Valid attribute mask */
				dir[DIR_Attr] = (value & mask) | (dir[DIR_Attr] & (BYTE)~mask);	/* Apply attribute change */
				res = sync(dj.fs);
			}
		}
	}
	return res;
}




/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_utime (
	const char *path,		/* Pointer to the file/directory name */
	const FILINFO *finfo	/* Pointer to the timestamp to be set */
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	char fn[8+3+1];


	res = auto_mount(&path, &dj.fs, 1);
	if (res == FR_OK) {
		res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
		if (res == FR_OK) {				/* Trace completed */
			if (!dir) {
				res = FR_INVALID_NAME;	/* Root directory */
			} else {
				ST_WORD(&dir[DIR_WrtTime], finfo->ftime);
				ST_WORD(&dir[DIR_WrtDate], finfo->fdate);
				res = sync(dj.fs);
			}
		}
	}
	return res;
}




/*-----------------------------------------------------------------------*/
/* Rename File/Directory                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_rename (
	const char *path_old,	/* Pointer to the old name */
	const char *path_new	/* Pointer to the new name */
)
{
	FRESULT res;
	DIR dj;
	DWORD sect_old;
	BYTE *dir_old, *dir_new, direntry[32-11];
	char fn[8+3+1];


	res = auto_mount(&path_old, &dj.fs, 1);
	if (res != FR_OK) return res;

	res = trace_path(&dj, fn, path_old, &dir_old);	/* Check old object */
	if (res != FR_OK) return res;				/* The old object is not found */
	if (!dir_old) return FR_NO_FILE;
	sect_old = dj.fs->winsect;					/* Save the object information */
	memcpy(direntry, &dir_old[DIR_Attr], 32-11);

	res = trace_path(&dj, fn, path_new, &dir_new);	/* Check new object */
	if (res == FR_OK) return FR_EXIST;			/* The new object name is already existing */
	if (res != FR_NO_FILE) return res;			/* Is there no old name? */
	res = reserve_direntry(&dj, &dir_new); 		/* Reserve a directory entry */
	if (res != FR_OK) return res;

	memcpy(&dir_new[DIR_Attr], direntry, 32-11);	/* Create new entry */
	memcpy(&dir_new[DIR_Name], fn, 8+3);
	dir_new[DIR_NTres] = fn[11];
	dj.fs->winflag = 1;

	if (!move_window(dj.fs, sect_old)) return FR_RW_ERROR;	/* Delete old entry */
	dir_old[DIR_Name] = 0xE5;

	return sync(dj.fs);
}

#endif /* !_FS_READONLY */
#endif /* _FS_MINIMIZE == 0 */
#endif /* _FS_MINIMIZE <= 1 */
#endif /* _FS_MINIMIZE <= 2 */



#if _USE_MKFS && !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Create File System on the Drive                                       */
/*-----------------------------------------------------------------------*/
#define N_ROOTDIR	512			/* Multiple of 32 and <= 2048 */
#define N_FATS		1			/* 1 or 2 */
#define MAX_SECTOR	64000000UL	/* Maximum partition size */
#define MIN_SECTOR	2000UL		/* Minimum partition size */



FRESULT f_mkfs (
	BYTE drv,			/* Logical drive number */
	BYTE partition,		/* Partitioning rule 0:FDISK, 1:SFD */
	WORD allocsize		/* Allocation unit size [bytes] */
)
{
	BYTE fmt, m, *tbl;
	DWORD b_part, b_fat, b_dir, b_data;		/* Area offset (LBA) */
	DWORD n_part, n_rsv, n_fat, n_dir;		/* Area size */
	DWORD n_clust, n;
	FATFS *fs;
	DSTATUS stat;


	/* Check validity of the parameters */
	if (drv >= _DRIVES) return FR_INVALID_DRIVE;
	if (partition >= 2) return FR_MKFS_ABORTED;
	for (n = 512; n <= 32768U && n != allocsize; n <<= 1);
	if (n != allocsize) return FR_MKFS_ABORTED;

	/* Check mounted drive and clear work area */
	fs = FatFs[drv];
	if (!fs) return FR_NOT_ENABLED;
	fs->fs_type = 0;
	drv = LD2PD(drv);

	/* Get disk statics */
	stat = disk_initialize(drv);
	if (stat & STA_NOINIT) return FR_NOT_READY;
	if (stat & STA_PROTECT) return FR_WRITE_PROTECTED;
	if (disk_ioctl(drv, GET_SECTOR_COUNT, &n_part) != RES_OK || n_part < MIN_SECTOR)
		return FR_MKFS_ABORTED;
	if (n_part > MAX_SECTOR) n_part = MAX_SECTOR;
	b_part = (!partition) ? 63 : 0;		/* Boot sector */
	n_part -= b_part;
#if S_MAX_SIZ > 512						/* Check disk sector size */
	if (disk_ioctl(drv, GET_SECTOR_SIZE, &SS(fs)) != RES_OK
		|| SS(fs) > S_MAX_SIZ
		|| SS(fs) > allocsize)
		return FR_MKFS_ABORTED;
#endif
	allocsize /= SS(fs);		/* Number of sectors per cluster */

	/* Pre-compute number of clusters and FAT type */
	n_clust = n_part / allocsize;
	fmt = FS_FAT12;
	if (n_clust >= 0xFF5) fmt = FS_FAT16;
	if (n_clust >= 0xFFF5) fmt = FS_FAT32;

	/* Determine offset and size of FAT structure */
	switch (fmt) {
	case FS_FAT12:
		n_fat = ((n_clust * 3 + 1) / 2 + 3 + SS(fs) - 1) / SS(fs);
		n_rsv = 1 + partition;
		n_dir = N_ROOTDIR * 32 / SS(fs);
		break;
	case FS_FAT16:
		n_fat = ((n_clust * 2) + 4 + SS(fs) - 1) / SS(fs);
		n_rsv = 1 + partition;
		n_dir = N_ROOTDIR * 32 / SS(fs);
		break;
	default:
		n_fat = ((n_clust * 4) + 8 + SS(fs) - 1) / SS(fs);
		n_rsv = 33 - partition;
		n_dir = 0;
	}
	b_fat = b_part + n_rsv;			/* FATs start sector */
	b_dir = b_fat + n_fat * N_FATS;	/* Directory start sector */
	b_data = b_dir + n_dir;			/* Data start sector */

	/* Align data start sector to erase block boundary (for flash memory media) */
	if (disk_ioctl(drv, GET_BLOCK_SIZE, &n) != RES_OK) return FR_MKFS_ABORTED;
	n = (b_data + n - 1) & ~(n - 1);
	n_fat += (n - b_data) / N_FATS;
	/* b_dir and b_data are no longer used below */

	/* Determine number of cluster and final check of validity of the FAT type */
	n_clust = (n_part - n_rsv - n_fat * N_FATS - n_dir) / allocsize;
	if (   (fmt == FS_FAT16 && n_clust < 0xFF5)
		|| (fmt == FS_FAT32 && n_clust < 0xFFF5))
		return FR_MKFS_ABORTED;

	/* Create partition table if needed */
	if (!partition) {
		DWORD n_disk = b_part + n_part;

		tbl = &fs->win[MBR_Table];
		ST_DWORD(&tbl[0], 0x00010180);	/* Partition start in CHS */
		if (n_disk < 63UL * 255 * 1024) {	/* Partition end in CHS */
			n_disk = n_disk / 63 / 255;
			tbl[7] = (BYTE)n_disk;
			tbl[6] = (BYTE)((n_disk >> 2) | 63);
		} else {
			ST_WORD(&tbl[6], 0xFFFF);
		}
		tbl[5] = 254;
		if (fmt != FS_FAT32)			/* System ID */
			tbl[4] = (n_part < 0x10000) ? 0x04 : 0x06;
		else
			tbl[4] = 0x0c;
		ST_DWORD(&tbl[8], 63);			/* Partition start in LBA */
		ST_DWORD(&tbl[12], n_part);		/* Partition size in LBA */
		ST_WORD(&tbl[64], 0xAA55);		/* Signature */
		if (disk_write(drv, fs->win, 0, 1) != RES_OK)
			return FR_RW_ERROR;
	}

	/* Create boot record */
	tbl = fs->win;								/* Clear buffer */
	memset(tbl, 0, SS(fs));
	ST_DWORD(&tbl[BS_jmpBoot], 0x90FEEB);		/* Boot code (jmp $, nop) */
	ST_WORD(&tbl[BPB_BytsPerSec], SS(fs));		/* Sector size */
	tbl[BPB_SecPerClus] = (BYTE)allocsize;		/* Sectors per cluster */
	ST_WORD(&tbl[BPB_RsvdSecCnt], n_rsv);		/* Reserved sectors */
	tbl[BPB_NumFATs] = N_FATS;					/* Number of FATs */
	ST_WORD(&tbl[BPB_RootEntCnt], SS(fs) / 32 * n_dir); /* Number of rootdir entries */
	if (n_part < 0x10000) {						/* Number of total sectors */
		ST_WORD(&tbl[BPB_TotSec16], n_part);
	} else {
		ST_DWORD(&tbl[BPB_TotSec32], n_part);
	}
	tbl[BPB_Media] = 0xF8;						/* Media descripter */
	ST_WORD(&tbl[BPB_SecPerTrk], 63);			/* Number of sectors per track */
	ST_WORD(&tbl[BPB_NumHeads], 255);			/* Number of heads */
	ST_DWORD(&tbl[BPB_HiddSec], b_part);		/* Hidden sectors */
	n = get_fattime();							/* Use current time as a VSN */
	if (fmt != FS_FAT32) {
		ST_DWORD(&tbl[BS_VolID], n);			/* Volume serial number */
		ST_WORD(&tbl[BPB_FATSz16], n_fat);		/* Number of secters per FAT */
		tbl[BS_DrvNum] = 0x80;					/* Drive number */
		tbl[BS_BootSig] = 0x29;					/* Extended boot signature */
		memcpy(&tbl[BS_VolLab], "NO NAME    FAT     ", 19);	/* Volume lavel, FAT signature */
	} else {
		ST_DWORD(&tbl[BS_VolID32], n);			/* Volume serial number */
		ST_DWORD(&tbl[BPB_FATSz32], n_fat);		/* Number of secters per FAT */
		ST_DWORD(&tbl[BPB_RootClus], 2);		/* Root directory cluster (2) */
		ST_WORD(&tbl[BPB_FSInfo], 1);			/* FSInfo record (bs+1) */
		ST_WORD(&tbl[BPB_BkBootSec], 6);		/* Backup boot record (bs+6) */
		tbl[BS_DrvNum32] = 0x80;				/* Drive number */
		tbl[BS_BootSig32] = 0x29;				/* Extended boot signature */
		memcpy(&tbl[BS_VolLab32], "NO NAME    FAT32   ", 19);	/* Volume lavel, FAT signature */
	}
	ST_WORD(&tbl[BS_55AA], 0xAA55);			/* Signature */
	if (disk_write(drv, tbl, b_part+0, 1) != RES_OK)
		return FR_RW_ERROR;
	if (fmt == FS_FAT32)
		disk_write(drv, tbl, b_part+6, 1);

	/* Initialize FAT area */
	for (m = 0; m < N_FATS; m++) {
		memset(tbl, 0, SS(fs));		/* 1st sector of the FAT  */
		if (fmt != FS_FAT32) {
			n = (fmt == FS_FAT12) ? 0x00FFFFF8 : 0xFFFFFFF8;
			ST_DWORD(&tbl[0], n);			/* Reserve cluster #0-1 (FAT12/16) */
		} else {
			ST_DWORD(&tbl[0], 0xFFFFFFF8);	/* Reserve cluster #0-1 (FAT32) */
			ST_DWORD(&tbl[4], 0xFFFFFFFF);
			ST_DWORD(&tbl[8], 0x0FFFFFFF);	/* Reserve cluster #2 for root dir */
		}
		if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
			return FR_RW_ERROR;
		memset(tbl, 0, SS(fs));		/* Following FAT entries are filled by zero */
		for (n = 1; n < n_fat; n++) {
			if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
				return FR_RW_ERROR;
		}
	}

	/* Initialize Root directory */
	m = (BYTE)((fmt == FS_FAT32) ? allocsize : n_dir);
	do {
		if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
			return FR_RW_ERROR;
	} while (--m);

	/* Create FSInfo record if needed */
	if (fmt == FS_FAT32) {
		ST_WORD(&tbl[BS_55AA], 0xAA55);
		ST_DWORD(&tbl[FSI_LeadSig], 0x41615252);
		ST_DWORD(&tbl[FSI_StrucSig], 0x61417272);
		ST_DWORD(&tbl[FSI_Free_Count], n_clust - 1);
		ST_DWORD(&tbl[FSI_Nxt_Free], 0xFFFFFFFF);
		disk_write(drv, tbl, b_part+1, 1);
		disk_write(drv, tbl, b_part+7, 1);
	}

	return (disk_ioctl(drv, CTRL_SYNC, NULL) == RES_OK) ? FR_OK : FR_RW_ERROR;
}

#endif /* _USE_MKFS && !_FS_READONLY */




#if _USE_STRFUNC >= 1
/*-----------------------------------------------------------------------*/
/* Get a string from the file                                            */
/*-----------------------------------------------------------------------*/
char* fgets (
	char* buff,	/* Pointer to the string buffer to read */
	int len,	/* Size of string buffer */
	FIL* fil	/* Pointer to the file object */
)
{
	int i = 0;
	char *p = buff;
	UINT rc;


	while (i < len - 1) {			/* Read bytes until buffer gets filled */
		f_read(fil, p, 1, &rc);
		if (rc != 1) break;			/* Break when no data to read */
#if _USE_STRFUNC >= 2
		if (*p == '\r') continue;	/* Strip '\r' */
#endif
		i++;
		if (*p++ == '\n') break;	/* Break when reached end of line */
	}
	*p = 0;
	return i ? buff : 0;			/* When no data read (eof or error), return with error. */
}



#if !_FS_READONLY
#include <stdarg.h>
/*-----------------------------------------------------------------------*/
/* Put a character to the file                                           */
/*-----------------------------------------------------------------------*/
int fputc (
	int chr,	/* A character to be output */
	FIL* fil	/* Ponter to the file object */
)
{
	UINT bw;
	char c;


#if _USE_STRFUNC >= 2
	if (chr == '\n') fputc ('\r', fil);	/* LF -> CRLF conversion */
#endif
	if (!fil) {	/* Special value may be used to switch the destination to any other device */
	/*	put_console(chr);	*/
		return chr;
	}
	c = (char)chr;
	f_write(fil, &c, 1, &bw);	/* Write a byte to the file */
	return bw ? chr : EOF;		/* Return the resulut */
}




/*-----------------------------------------------------------------------*/
/* Put a string to the file                                              */
/*-----------------------------------------------------------------------*/
int fputs (
	const char* str,	/* Pointer to the string to be output */
	FIL* fil			/* Pointer to the file object */
)
{
	int n;


	for (n = 0; *str; str++, n++) {
		if (fputc(*str, fil) == EOF) return EOF;
	}
	return n;
}




/*-----------------------------------------------------------------------*/
/* Put a formatted string to the file                                    */
/*-----------------------------------------------------------------------*/
int fprintf (
	FIL* fil,			/* Pointer to the file object */
	const char* str,	/* Pointer to the format string */
	...					/* Optional arguments... */
)
{
	va_list arp;
	UCHAR c, f, r;
	ULONG val;
	char s[16];
	int i, w, res, cc;


	va_start(arp, str);

	for (cc = res = 0; cc != EOF; res += cc) {
		c = *str++;
		if (c == 0) break;			/* End of string */
		if (c != '%') {				/* Non escape cahracter */
			cc = fputc(c, fil);
			if (cc != EOF) cc = 1;
			continue;
		}
		w = f = 0;
		c = *str++;
		if (c == '0') {				/* Flag: '0' padding */
			f = 1; c = *str++;
		}
		while (c >= '0' && c <= '9') {	/* Precision */
			w = w * 10 + (c - '0');
			c = *str++;
		}
		if (c == 'l') {				/* Prefix: Size is long int */
			f |= 2; c = *str++;
		}
		if (c == 's') {				/* Type is string */
			cc = fputs(va_arg(arp, char*), fil);
			continue;
		}
		if (c == 'c') {				/* Type is character */
			cc = fputc(va_arg(arp, char), fil);
			if (cc != EOF) cc = 1;
			continue;
		}
		r = 0;
		if (c == 'd') r = 10;		/* Type is signed decimal */
		if (c == 'u') r = 10;		/* Type is unsigned decimal */
		if (c == 'X') r = 16;		/* Type is unsigned hexdecimal */
		if (r == 0) break;			/* Unknown type */
		if (f & 2) {				/* Get the value */
			val = (ULONG)va_arg(arp, long);
		} else {
			val = (c == 'd') ? (ULONG)(long)va_arg(arp, int) : (ULONG)va_arg(arp, unsigned int);
		}
		/* Put numeral string */
		if (c == 'd') {
			if (val >= 0x80000000) {
				val = 0 - val;
				f |= 4;
			}
		}
		i = sizeof(s) - 1; s[i] = 0;
		do {
			c = (UCHAR)(val % r + '0');
			if (c > '9') c += 7;
			s[--i] = c;
			val /= r;
		} while (i && val);
		if (i && (f & 4)) s[--i] = '-';
		w = sizeof(s) - 1 - w;
		while (i && i > w) s[--i] = (f & 1) ? '0' : ' ';
		cc = fputs(&s[i], fil);
	}

	va_end(arp);
	return (cc == EOF) ? cc : res;
}

#endif /* !_FS_READONLY */
#endif /* _USE_STRFUNC >= 1*/
