/*
 * niffs.h
 *
 *  Created on: Feb 2, 2015
 *      Author: petera
 */

#ifndef NIFFS_H_
#define NIFFS_H_

#include "niffs_config.h"

#define NIFFS_SEEK_SET          (0)
#define NIFFS_SEEK_CUR          (1)
#define NIFFS_SEEK_END          (2)

/* Any write to the filehandle is appended to end of the file */
#define NIFFS_O_APPEND                      (1<<0)
/* If the opened file exists, it will be truncated to zero length before opened */
#define NIFFS_O_TRUNC                       (1<<1)
/* If the opened file does not exist, it will be created before opened */
#define NIFFS_O_CREAT                       (1<<2)
/* The opened file may only be read */
#define NIFFS_O_RDONLY                      (1<<3)
/* The opened file may only be writted */
#define NIFFS_O_WRONLY                      (1<<4)
/* The opened file may be both read and written */
#define NIFFS_O_RDWR                        (NIFFS_O_RDONLY | NIFFS_O_WRONLY)
/* Any writes to the filehandle will never be cached */
#define NIFFS_O_DIRECT                      (1<<5)
/* If O_CREAT and O_EXCL are set, open() fails if the file exists. */
#define NIFFS_O_EXCL                        (1<<6)
/* If O_CREAT and O_LINEAR is enabled, along with config NIFFS_LINEAR_AREA,
   the created file will be put in the linear area */
#define NIFFS_O_LINEAR                      (1<<7)

#ifndef NIFFS_ERR_BASE
#define NIFFS_ERR_BASE                      (11000)
#endif

#define NIFFS_OK                            0
#define ERR_NIFFS_BAD_CONF                  -(NIFFS_ERR_BASE + 1)
#define ERR_NIFFS_NOT_A_FILESYSTEM          -(NIFFS_ERR_BASE + 2)
#define ERR_NIFFS_BAD_SECTOR                -(NIFFS_ERR_BASE + 3)
#define ERR_NIFFS_DELETING_FREE_PAGE        -(NIFFS_ERR_BASE + 4)
#define ERR_NIFFS_DELETING_DELETED_PAGE     -(NIFFS_ERR_BASE + 5)
#define ERR_NIFFS_MOVING_FREE_PAGE          -(NIFFS_ERR_BASE + 6)
#define ERR_NIFFS_MOVING_DELETED_PAGE       -(NIFFS_ERR_BASE + 7)
#define ERR_NIFFS_MOVING_TO_UNFREE_PAGE     -(NIFFS_ERR_BASE + 8)
#define ERR_NIFFS_MOVING_TO_SAME_PAGE       -(NIFFS_ERR_BASE + 9)
#define ERR_NIFFS_MOVING_BAD_FLAG           -(NIFFS_ERR_BASE + 10)
#define ERR_NIFFS_NO_FREE_PAGE              -(NIFFS_ERR_BASE + 11)
#define ERR_NIFFS_SECTOR_UNFORMATTABLE      -(NIFFS_ERR_BASE + 12)
#define ERR_NIFFS_NULL_PTR                  -(NIFFS_ERR_BASE + 13)
#define ERR_NIFFS_NO_FREE_ID                -(NIFFS_ERR_BASE + 14)
#define ERR_NIFFS_WR_PHDR_UNFREE_PAGE       -(NIFFS_ERR_BASE + 15)
#define ERR_NIFFS_WR_PHDR_BAD_ID            -(NIFFS_ERR_BASE + 16)
#define ERR_NIFFS_NAME_CONFLICT             -(NIFFS_ERR_BASE + 17)
#define ERR_NIFFS_FULL                      -(NIFFS_ERR_BASE + 18)
#define ERR_NIFFS_OUT_OF_FILEDESCS          -(NIFFS_ERR_BASE + 19)
#define ERR_NIFFS_FILE_NOT_FOUND            -(NIFFS_ERR_BASE + 20)
#define ERR_NIFFS_FILEDESC_CLOSED           -(NIFFS_ERR_BASE + 21)
#define ERR_NIFFS_FILEDESC_BAD              -(NIFFS_ERR_BASE + 22)
#define ERR_NIFFS_INCOHERENT_ID             -(NIFFS_ERR_BASE + 23)
#define ERR_NIFFS_PAGE_NOT_FOUND            -(NIFFS_ERR_BASE + 24)
#define ERR_NIFFS_END_OF_FILE               -(NIFFS_ERR_BASE + 25)
#define ERR_NIFFS_MODIFY_BEYOND_FILE        -(NIFFS_ERR_BASE + 26)
#define ERR_NIFFS_TRUNCATE_BEYOND_FILE      -(NIFFS_ERR_BASE + 27)
#define ERR_NIFFS_NO_GC_CANDIDATE           -(NIFFS_ERR_BASE + 28)
#define ERR_NIFFS_PAGE_DELETED              -(NIFFS_ERR_BASE + 29)
#define ERR_NIFFS_PAGE_FREE                 -(NIFFS_ERR_BASE + 30)
#define ERR_NIFFS_MOUNTED                   -(NIFFS_ERR_BASE + 31)
#define ERR_NIFFS_NOT_MOUNTED               -(NIFFS_ERR_BASE + 32)
#define ERR_NIFFS_NOT_WRITABLE              -(NIFFS_ERR_BASE + 33)
#define ERR_NIFFS_NOT_READABLE              -(NIFFS_ERR_BASE + 34)
#define ERR_NIFFS_FILE_EXISTS               -(NIFFS_ERR_BASE + 35)
#define ERR_NIFFS_OVERFLOW                  -(NIFFS_ERR_BASE + 36)
#define ERR_NIFFS_LINEAR_FILE               -(NIFFS_ERR_BASE + 37)
#define ERR_NIFFS_LINEAR_NO_SPACE           -(NIFFS_ERR_BASE + 38)

typedef int (* niffs_hal_erase_f)(u8_t *addr, u32_t len);
typedef int (* niffs_hal_write_f)(u8_t *addr, const u8_t *src, u32_t len);
// dummy type, for posix compliance
typedef u16_t niffs_mode;
// niffs file descriptor flags
typedef u8_t niffs_fd_flags;
// niffs file type
typedef u8_t niffs_file_type;

/* file descriptor */
typedef struct {
  // object id
  niffs_obj_id obj_id;
  // page index for object index
  niffs_page_ix obj_pix;
  // file type
  niffs_file_type type;
  // file descriptor offset
  u32_t offs;
  // page index for current file desc offset
  niffs_page_ix cur_pix;
  // file descriptor flags
  niffs_fd_flags flags;
} niffs_file_desc;

/* fs struct */
typedef struct {
  /* static cfg */
  // physical address where fs resides
  u8_t *phys_addr;
  // number of logical sectors
  u32_t sectors;
  // logical sector size in bytes
  u32_t sector_size;
  // logical page size in bytes
  u32_t page_size;
#if NIFFS_LINEAR_AREA
  // number of linear sectors
  u32_t lin_sectors;
#endif

  // work buffer
  u8_t *buf;
  // work buffer length
  u32_t buf_len;

  // HAL write function
  niffs_hal_write_f hal_wr;
  // HAL erase function
  niffs_hal_erase_f hal_er;

  /* dynamics */
  // pages per sector
  u32_t pages_per_sector;
  // last seen free page index
  niffs_page_ix last_free_pix;
  // whether mounted or not
  u8_t mounted;
  // number of free pages
  u32_t free_pages;
  // number of deleted pages
  u32_t dele_pages;
  // file descriptor array
  niffs_file_desc *descs;
  // number of file descriptors
  u32_t descs_len;
  // max erase count
  niffs_erase_cnt max_era;
} niffs;

/* niffs file status struct */
typedef struct {
  // file object id
  niffs_obj_id obj_id;
  // file size
  u32_t size;
  // file name
  u8_t name[NIFFS_NAME_LEN];
  // file type
  niffs_file_type type;
} niffs_stat;

/* niffs file directory entry struct */
struct niffs_dirent {
  // file object id
  niffs_obj_id obj_id;
  // file name
  u8_t name[NIFFS_NAME_LEN];
  // file size
  u32_t size;
  // file index header whereabouts
  niffs_page_ix pix;
  // file type
  niffs_file_type type;
};

/*  niffs file directory struct */
typedef struct {
  // the actual fs
  niffs *fs;
  // current search page index
  niffs_page_ix pix;
} niffs_DIR;

/* niffs fs info struct */
typedef struct {
  /* total amount of bytes in filesystem (linear parts excluded) */
  s32_t total_bytes;
  /* used bytes in filesystem (linear parts excluded) */
  s32_t used_bytes;
  /* If non-zero, this means you should delete some files and run a check.
   This can happen if filesystem loses power repeatedly during
   garbage collection or check. */
  u8_t overflow;

  /* total amount of sectors in the linear part of filesystem */
  s32_t lin_total_sectors;
  /* used sectors in the linear part of filesystem */
  s32_t lin_used_sectors;
  /* maximum free consecutive area in the linear part of filesystem */
  s32_t lin_max_conseq_free;
} niffs_info;

/**
 * Initializes and configures the file system.
 * The file system needs a ram work buffer being at least a logical page size
 * big. In some cases, this needs to be extended. NIFFS will return
 * ERR_NIFFS_BAD_CONF on bad configurations. If NIFFS_DBG is enabled, a
 * descriptive message will also tell you what's wrong.
 *
 * @param fs            the file system struct
 * @param phys_addr     the starting address of the filesystem on flash
 * @param sectors       number of sectors comprised by the filesystem
 * @param sector_size   logical sector size
 * @param page_size     logical page size
 * @param buf           ram work buffer
 * @param buf_len       ram work buffer length
 * @param descs         ram file descriptor buffer
 * @param file_desc_len number of file descriptors in buffer
 * @param erase_f       HAL erase function
 * @param write_f       HAL write function
 * @param lin_sectors   Ignored if NIFFS_LINEAR_AREA is 0. Otherwise, allocates
 *                      lin_sectors of space for the linear area. This space
 *                      will be allotted after the dynamic fs.
 */
int NIFFS_init(niffs *fs,
    u8_t *phys_addr,
    u32_t sectors,
    u32_t sector_size,
    u32_t page_size,
    u8_t *buf,
    u32_t buf_len,
    niffs_file_desc *descs,
    u32_t file_desc_len,
    niffs_hal_erase_f erase_f,
    niffs_hal_write_f write_f,
    u32_t lin_sectors
    );

/**
 * Mounts the filesystem
 * @param fs            the file system struct
 */
int NIFFS_mount(niffs *fs);

/**
 * Returns some general info
 * @param fs            the file system struct
 * @param total         will be populated with total amount of bytes in filesystem
 * @param used          will be populated with used bytes in filesystem
 * @param overflow      if !0, this means you should delete some files and run a check.
 *                      This can happen if filesystem loses power repeatedly during
 *                      garbage collection or check.
 */
int NIFFS_info(niffs *fs, niffs_info *i);

/**
 * Creates a new file.
 * @param fs            the file system struct
 * @param name          the name of the new file
 * @param mode          ignored, for posix compliance
 */
int NIFFS_creat(niffs *fs, const char *name, niffs_mode mode);

#if NIFFS_LINEAR_AREA
/**
 * Creates a new file in the linear area.
 * @param fs            the file system struct
 * @param name          the name of the new file
 * @param resv_size     Hint to the filesystem how large this file will be in
 *                      bytes. May be 0 if not known.
 *                      As linear files cannot be chunked up by pages, they
 *                      will be placed after each other on medium. For example,
 *                      when creating linear file A it will be placed on
 *                      sector x. If creating linear file B directly afterwards,
 *                      B will be placed on sector x+1. This constricts file A
 *                      to grow one sector only. However, if A is created with
 *                      resv_size of 10 sectors, B will be created on sector
 *                      x+10, giving A room to grow 10 sectors instead.
 * @return file descriptor with flags O_LINEAR | O_RDWR | O_APPEND or error
 */
int NIFFS_mknod_linear(niffs *fs, const char *name, u32_t resv_size);
#endif

/**
 * Opens/creates a file.
 * @param fs            the file system struct
 * @param path          the path of the new file
 * @param flags         the flags for the open command, can be combinations of
 *                      NIFFS_O_APPEND, NIFFS_O_TRUNC, NIFFS_O_CREAT, NIFFS_O_RDONLY,
 *                      NIFFS_O_WRONLY, NIFFS_O_RDWR, NIFFS_O_DIRECT, NIFFS_O_LINEAR
 * @param mode          ignored, for posix compliance
 * @return file descriptor or error
 *
 * Note: when creating files with NIFFS_O_LINEAR, NIFFS_O_APPEND is
 * automatically set. Linear files cannot be modified, only appended.
 */
int NIFFS_open(niffs *fs, const char *name, u8_t flags, niffs_mode mode);

/**
 * Returns a pointer directly to the flash where data resides, and how many
 * bytes which can be read.
 * This function does not advance the file descriptor offset, so NIFFS_lseek
 * should be called prior to NIFFS_read_ptr.
 * @param fs            the file system struct
 * @param fd            the filehandle
 * @param ptr           ptr which is populated with adress to data
 * @param len           populated with valid data length
 */
int NIFFS_read_ptr(niffs *fs, int fd, u8_t **ptr, u32_t *len);

/**
 * Reads from given filehandle.
 * NB: consider using NIFFS_read_ptr instead. This will basically copy from your
 * internal flash to your ram. If you're only interested in reading data and not
 * modifying it, this will basically waste cycles and ram on memcpy.
 * @param fs            the file system struct
 * @param fd            the filehandle
 * @param buf           where to put read data
 * @param len           how much to read
 * @returns number of bytes read, or error
 */
int NIFFS_read(niffs *fs, int fd, u8_t *dst, u32_t len);

/**
 * Moves the read/write file offset
 * @param fs            the file system struct
 * @param fh            the filehandle
 * @param offs          how much/where to move the offset
 * @param whence        if NIFFS_SEEK_SET, the file offset shall be set to offset bytes
 *                      if NIFFS_SEEK_CUR, the file offset shall be set to its current location plus offset
 *                      if NIFFS_SEEK_END, the file offset shall be set to the size of the file plus offset
 */
int NIFFS_lseek(niffs *fs, int fd, s32_t offs, int whence);

/**
 * Removes a file by name
 * @param fs            the file system struct
 * @param name          the name of the file to remove
 */
int NIFFS_remove(niffs *fs, const char *name);

/**
 * Removes a file by filehandle
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to remove
 */
int NIFFS_fremove(niffs *fs, int fd);

/**
 * Writes to given filehandle.
 * @param fs            the file system struct
 * @param fd            the filehandle
 * @param buf           the data to write
 * @param len           how much to write
 * @returns number of bytes written or error
 */
int NIFFS_write(niffs *fs, int fd, const u8_t *data, u32_t len);

/**
 * Flushes all pending write operations from cache for given file
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to flush
 */
int NIFFS_fflush(niffs *fs, int fd);

/**
 * Gets file status by name
 * @param fs            the file system struct
 * @param path          the name of the file to stat
 * @param s             the stat struct to populate
 */
int NIFFS_stat(niffs *fs, const char *name, niffs_stat *s);

/**
 * Gets file status by filehandle
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to stat
 * @param s             the stat struct to populate
 */
int NIFFS_fstat(niffs *fs, int fd, niffs_stat *s);

/**
 * Gets current position in stream
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to return position from
 */
int NIFFS_ftell(niffs *fs, int fd);

/**
 * Closes a filehandle. If there are pending write operations, these are finalized before closing.
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to close
 */
int NIFFS_close(niffs *fs, int fd);

/**
 * Renames a file.
 * @param fs            the file system struct
 * @param old           name of file to rename
 * @param new           new name of file
 */
int NIFFS_rename(niffs *fs, const char *old_name, const char *new_name);

/**
 * Opens a directory stream corresponding to the given name.
 * The stream is positioned at the first entry in the directory.
 * The name argument is ignored as hydrogen builds always correspond
 * to a flat file structure - no directories.
 * @param fs            the file system struct
 * @param name          the name of the directory
 * @param d             pointer the directory stream to be populated
 */
niffs_DIR *NIFFS_opendir(niffs *fs, const char *name, niffs_DIR *d);

/**
 * Closes a directory stream
 * @param d             the directory stream to close
 */
int NIFFS_closedir(niffs_DIR *d);

/**
 * Reads a directory into given niffs_dirent struct.
 * @param d             pointer to the directory stream
 * @param e             the dirent struct to be populated
 * @returns null if error or end of stream, else given dirent is returned
 */
struct niffs_dirent *NIFFS_readdir(niffs_DIR *d, struct niffs_dirent *e);

/**
 * Unmounts the file system. All file handles will be flushed of any
 * cached writes and closed.
 * @param fs            the file system struct
 */
int NIFFS_unmount(niffs *fs);

/**
 * Formats the entire filesystem.
 * @param fs            the file system struct
 */
int NIFFS_format(niffs *fs);

/**
 * Runs a consistency check on given filesystem and mends any aborted operations.
 * @param fs            the file system struct
 */
int NIFFS_chk(niffs *fs);

#ifdef NIFFS_DUMP
/**
 * Prints out a visualization of the filesystem.
 * @param fs            the file system struct
 */
void NIFFS_dump(niffs *fs);
#endif

#endif /* NIFFS_H_ */
