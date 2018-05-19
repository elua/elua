/*
 * niffs_internal.h
 *
 * FLASH LAYOUT
 * SECTOR:
 *  <sector header size> <page size>              <page size>
 *  | niffs_sector_hdr  | niffs_page_hdr : data | niffs_page_hdr : data | ... | fill |
 *  | magic : era_cnt   | id   :  flag   | .... | id   :  flag   | .... | ...
 *  |                   | \ obj_id              | \ obj_id              |
 *  |                   | \ spix                | \ spix                |
 *
 * Page status:
 *   FREE  niffs_page_hdr.id      0xff..
 *   DELE  niffs_page_hdr.id      0x0
 *   USED  niffs_page_hdr.id      != 0 && != 0xff..
 *   CLEA  niffs_page_hdr.flag    0xff..
 *   WRIT  niffs_page_hdr.flag    0x1
 *   MOVI  niffs_page_hdr.flag    0x0
 *
 * Free pages:
 *   FREE & CLEA
 * Allocated pages:
 *   USED
 *   USED & CLEA : allocated but not yet written (semi bad)
 *   USED & WRIT : allocated and written
 *   USED & MOVI : allocated and being moved/updated
 * Bad pages:
 *   FREE & WRIT : not allowed
 *   FREE & MOVI : not allowed
 *
 * A page life cycle: FREE->CLEAN(->WRITN(->MOVIN))->DELE->FREE
 *
 * Object header:
 *         niffs_page_hdr.id      != 0 && != 0xff..
 *         niffs_page_hdr.id.spix 0
 *
 * Invalid object_ids are 0 and 0xff..
 *
 * File creation:
 *   write phdr.flag
 *   write phdr.id
 *   ----- page now allocated
 *   write ohdr (len = -1)
 *   ----- file now created, size 0
 *
 * File open:
 *   * aborted append / updated length:
 *   If obj header is MOVI:
 *   try finding a WRIT obj header with id,
 *   use this if found, delete MOVI. Else, copy MOVI as WRIT and
 *   delete MOVI. For used obj header:  check if there are more spix
 *   pages than file len allows, delete these

 * File append:
 *   if spix == 0 & len == -1, use same page, update page after full/finished as WRIT
 *   for following pages:
 *     mark obj hdr as MOVI
 *     if free space in last page
 *       mark MOVI, fill copy, write WRIT, delete MOVI
 *     create new pages with new spix as WRIT
 *     store new obj hdr with correct len as WRIT
 *     delete obj hdr MOVI
 *
 * File read:
 *   * aborted rewrite / updated content:
 *   if a MOVI page is encountered, niffs must check for pages
 *   WRIT with same id/span index. If found, use this page instead
 *   and delete MOVI. Else, copy MOVI page to free page as WRIT,
 *   delete MOVI page, and use dest.
 *
 * File trunc:
 *   trunc to zero/rm:
 *      set obj hdr file len to 0
 *      delete all pages with same id
 *      if aborted, check will clean
 *      delete obj hdr
 *   trunc to size:
 *      mark obj hdr MOVi
 *      create new obj hdr with correct size as WRIT
 *      if aborted, check will clean
 *      rewrite last page if trunc to midst of page (MOVI -> WRIT -> delete)
 *      delete all removed pages
 *
 * Check:
 *   sector headers for valid magic
 *
 *   if FREE & (WRIT | MOVI) bad page, delete
 *   if MOVI and no equal page with same id, copy and delete original
 *   if MOVI object header => modified during trunc or append, check length and all spix with same id
 *   file length - remove all pages where spix is beyond file size
 *
 *   remove orphans - check for WRIT && MOVI with ids and spix > 0 having no corresponding page with spix == 0, remove all
 *   file length - if length == 0, remove all with same id, including obj hdr
 *   if obj hdr flag is WRIT, but length is UNDEF, fail during append, remove all containing same id
 *
 *  Created on: Feb 3, 2015
 *      Author: petera
 */

#include "niffs.h"
#include "niffs_config.h"

#ifndef NIFFS_INTERNAL_H_
#define NIFFS_INTERNAL_H_

#define _NIFFS_PAGE_FREE_ID     ((niffs_page_id_raw)-1)
#define _NIFFS_PAGE_DELE_ID     ((niffs_page_id_raw)0)

#define _NIFFS_FLAG_CLEAN       ((niffs_flag)-1)
#define _NIFFS_FLAG_WRITTEN     ((niffs_flag)1)
#define _NIFFS_FLAG_MOVING      ((niffs_flag)0)
#define NIFFS_FLAG_MOVE_KEEP    ((niffs_flag)0xaa)

#define _NIFFS_FTYPE_FILE       (0)
#define _NIFFS_FTYPE_LINFILE    (1)

// change of magic since file type introduction
#if NIFFS_LINEAR_AREA
  //If we are using a partially linear file system, we need to store the linear vs regular page allocations
  //somewhere so lets store it in the first 4 bytes of the sector magic!
  //LINEAR MAGIC SECTOR DEFINITION
  //BYTES 7-6 (0xe1)
  //BYTES 5-3 fs->page_size
  //BYTES 2-0 fs->linear sectors 
  #define _NIFFS_SECT_MAGIC_BYTES 0xe1
  #define _NIFFS_SECT_MAGIC(_fs)  (niffs_magic)((_NIFFS_SECT_MAGIC_BYTES << 24) | (((_fs)->page_size & 0xFFF) << 12) | ((_fs)->lin_sectors & 0xFFF)) 
  //Bytes 5-3 of magic number holds the page size
  #define _NIFFS_SECT_PAGE_SIZE(data)  (u32_t)((data >> 12) & 0xFFF) 
  //Lower 3 bytes of magic number holds the linear size
  #define _NIFFS_SECT_LINEAR_SIZE(data)  (u32_t)(data & 0xFFF) 
#else
  #define _NIFFS_SECT_MAGIC(_fs)  (niffs_magic)(0xfee1c001 ^ (_fs)->page_size)
#endif 

#ifndef _NIFFS_ALIGN
#define _NIFFS_ALIGN __attribute__ (( aligned(NIFFS_WORD_ALIGN) ))
#endif
#ifndef _NIFFS_PACKED
#define _NIFFS_PACKED __attribute__ (( packed ))
#endif

// checks if page is free, i.e. not used at all
#define _NIFFS_IS_FREE(_pg_hdr) (((_pg_hdr)->id.raw) == _NIFFS_PAGE_FREE_ID)
// checks if page is deleted
#define _NIFFS_IS_DELE(_pg_hdr) (((_pg_hdr)->id.raw) == _NIFFS_PAGE_DELE_ID)
// checks if page is allocated, but has no data written to it
#define _NIFFS_IS_CLEA(_pg_hdr) (((_pg_hdr)->flag) == _NIFFS_FLAG_CLEAN)
// checks if page is written
#define _NIFFS_IS_WRIT(_pg_hdr) (((_pg_hdr)->flag) == _NIFFS_FLAG_WRITTEN)
// checks if page is moving
#define _NIFFS_IS_MOVI(_pg_hdr) (((_pg_hdr)->flag) == _NIFFS_FLAG_MOVING)

#define _NIFFS_SECTOR_2_ADDR(_fs, _s) \
  ((_fs)->phys_addr + (_fs)->sector_size * (_s))
#define _NIFFS_ADDR_2_SECTOR(_fs, _addr) \
  (((u8_t *)(_addr) - (_fs)->phys_addr) / (_fs)->sector_size)

#define _NIFFS_PIX_2_SECTOR(_fs, _pix) \
  ((_pix) / (_fs)->pages_per_sector)
#define _NIFFS_PIX_IN_SECTOR(_fs, _pix) \
  ((_pix) % (_fs)->pages_per_sector)
#define _NIFFS_PIX_AT_SECTOR(_fs, _s) \
  ((_s) * (_fs)->pages_per_sector)
#define _NIFFS_PIX_2_ADDR(_fs, _pix) (\
  _NIFFS_SECTOR_2_ADDR(_fs, _NIFFS_PIX_2_SECTOR(_fs, _pix)) +  \
  sizeof(niffs_sector_hdr) + \
  _NIFFS_PIX_IN_SECTOR(_fs, _pix) * (_fs)->page_size \
  )

#define _NIFFS_SPIX_2_PDATA_LEN(_fs, _spix) \
  ((_fs)->page_size - sizeof(niffs_page_hdr) - ((_spix) == 0 ? sizeof(niffs_object_hdr) : 0))

#define _NIFFS_OFFS_2_SPIX(_fs, _offs) ( \
  (_offs) < _NIFFS_SPIX_2_PDATA_LEN(_fs, 0) ? 0 : \
      (1 + (((_offs) - _NIFFS_SPIX_2_PDATA_LEN(_fs, 0)) / _NIFFS_SPIX_2_PDATA_LEN(_fs, 1))) \
)

#define _NIFFS_OFFS_2_PDATA_OFFS(_fs, _offs) ( \
  (_offs) < _NIFFS_SPIX_2_PDATA_LEN(_fs, 0) ? (_offs) : \
      (((_offs) - _NIFFS_SPIX_2_PDATA_LEN(_fs, 0)) % _NIFFS_SPIX_2_PDATA_LEN(_fs, 1)) \
)

#define _NIFFS_RD(_fs, _dst, _src, _len) do {niffs_memcpy((_dst), (_src), (_len));}while(0)

#define _NIFFS_IS_ID_VALID(phdr) ((phdr)->id.obj_id != (niffs_obj_id)-1 && (phdr)-> id.obj_id != 0)
#define _NIFFS_IS_FLAG_VALID(phdr) \
  ((phdr)->flag == _NIFFS_FLAG_CLEAN || (phdr)->flag == _NIFFS_FLAG_WRITTEN || (phdr)->flag == _NIFFS_FLAG_MOVING)
#define _NIFFS_IS_OBJ_HDR(phdr) (_NIFFS_IS_ID_VALID(phdr) && (phdr->id.spix) == 0)

#define NIFFS_EXCL_SECT_NONE  (u32_t)-1
#define NIFFS_UNDEF_LEN       (u32_t)-1

#ifndef niffs_memcpy
#define niffs_memcpy(_d, _s, _l) memcpy((_d), (_s), (_l))
#endif
#ifndef niffs_memset
#define niffs_memset(_d, _v, _l) memset((_d), (_v), (_l))
#endif
#ifndef niffs_strncpy
#define niffs_strncpy(_d, _s, _l) strncpy((_d), (_s), (_l))
#endif

typedef struct {
  _NIFFS_ALIGN niffs_erase_cnt era_cnt;
  _NIFFS_ALIGN niffs_magic abra; // page size xored with magic
} _NIFFS_PACKED niffs_sector_hdr;

typedef struct {
  union {
    niffs_page_id_raw raw;
    struct {
      niffs_span_ix spix : NIFFS_SPAN_IX_BITS;
      niffs_obj_id obj_id : NIFFS_OBJ_ID_BITS;
    };
  };
} _NIFFS_PACKED niffs_page_hdr_id;

// keep member order, used in offsetof in internals
typedef struct {
  _NIFFS_ALIGN niffs_page_hdr_id id;
  _NIFFS_ALIGN niffs_flag flag;
} _NIFFS_PACKED niffs_page_hdr;

// keep member order, used in offsetof in internals
typedef struct {
  niffs_page_hdr phdr;
  _NIFFS_ALIGN u32_t len;
  _NIFFS_ALIGN u8_t name[NIFFS_NAME_LEN];
  _NIFFS_ALIGN niffs_file_type type;
}  _NIFFS_PACKED niffs_object_hdr;

typedef struct {
  niffs_object_hdr ohdr;
  _NIFFS_ALIGN u32_t start_sector; // absolute index from fs start
  _NIFFS_ALIGN u32_t resv_sectors;
} _NIFFS_PACKED niffs_linear_file_hdr;

// super header containing all header types
typedef union {
  niffs_page_hdr_id phdr;
  niffs_object_hdr ohdr;
  niffs_linear_file_hdr lfhdr;
  // .. add more if needed
} niffs_super_hdr;

#define NIFFS_VIS_CONT        1
#define NIFFS_VIS_END         2

typedef int (* niffs_visitor_f)(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg);

#ifdef NIFFS_TEST
TESTATIC int niffs_find_free_id(niffs *fs, niffs_obj_id *id, const char *conflict_name);
TESTATIC int niffs_find_free_page(niffs *fs, niffs_page_ix *pix, u32_t excl_sector);
TESTATIC int niffs_find_page(niffs *fs, niffs_page_ix *pix, niffs_obj_id oid, niffs_span_ix spix, niffs_page_ix start_pix);
TESTATIC int niffs_erase_sector(niffs *fs, u32_t sector_ix);
TESTATIC int niffs_move_page(niffs *fs, niffs_page_ix src_pix, niffs_page_ix dst_pix, const u8_t *data, u32_t len, niffs_flag force_flag);
TESTATIC int niffs_write_page(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, const u8_t *data, u32_t len);
TESTATIC int niffs_write_phdr(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr);
TESTATIC int niffs_delete_page(niffs *fs, niffs_page_ix pix);
#endif

int niffs_traverse(niffs *fs, niffs_page_ix pix_start, niffs_page_ix pix_end, niffs_visitor_f v, void *v_arg);
int niffs_get_filedesc(niffs *fs, int fd_ix, niffs_file_desc **fd);
int niffs_create(niffs *fs, const char *name, niffs_file_type type, void *meta);
int niffs_open(niffs *fs, const char *name, niffs_fd_flags flags);
int niffs_close(niffs *fs, int fd_ix);
int niffs_read_ptr(niffs *fs, int fd_ix, u8_t **data, u32_t *avail);
int niffs_seek(niffs *fs, int fd_ix, s32_t offset, u8_t whence);
int niffs_append(niffs *fs, int fd_ix, const u8_t *src, u32_t len);
int niffs_modify(niffs *fs, int fd_ix, u32_t offs, const u8_t *src, u32_t len);
int niffs_truncate(niffs *fs, int fd_ix, u32_t new_len);
int niffs_rename(niffs *fs, const char *old_name, const char *new_name);

int niffs_gc(niffs *fs, u32_t *freed_pages, u8_t allow_full_pages);

int niffs_chk(niffs *fs);

int niffs_linear_map(niffs *fs);
int niffs_linear_find_space(niffs *fs, u32_t sectors, u32_t *start_sector);
int niffs_linear_avail_size(niffs *fs, int fd_ix, u32_t *available_sectors);

#endif /* NIFFS_INTERNAL_H_ */
