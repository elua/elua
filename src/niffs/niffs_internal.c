#include "niffs_internal.h"

static int niffs_ensure_free_pages(niffs *fs, u32_t pages);
static int niffs_setup(niffs *fs);
static int niffs_chk_tidy_movi_objhdr_page(niffs *fs, niffs_page_ix pix, niffs_page_ix *dst_pix);

#define ERA_CNT_MIN_OF_LIMIT (((niffs_erase_cnt)-1)/4+1)
#define ERA_CNT_MAX_OF_LIMIT (((niffs_erase_cnt)-1)-ERA_CNT_MIN_OF_LIMIT+1)

#define check(_r) do { \
  int __res = (_r); \
  if (__res) { \
    NIFFS_DBG("err %i @ %s in %s:%i\n", __res, __func__, __FILE__, __LINE__); \
    return __res; \
  }\
}while(0);

TESTATIC int niffs_delete_page(niffs *fs, niffs_page_ix pix);
//////////////////////////////////// BASE ////////////////////////////////////

int niffs_traverse(niffs *fs, niffs_page_ix pix_start, niffs_page_ix pix_end, niffs_visitor_f v, void *v_arg) {
  int res = NIFFS_OK;
  int v_res = NIFFS_OK;
  niffs_page_ix pix = pix_start;
  if (pix >= fs->pages_per_sector * fs->sectors) {
    pix = 0;
    if (pix == pix_end) return NIFFS_VIS_END;
  }
  do {
    niffs_page_hdr *phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
    v_res = v(fs, pix, phdr, v_arg);
    if (v_res != NIFFS_VIS_CONT) {
      res = v_res;
      break;
    }
    // next page, wrap if necessary
    pix++;
    if (pix >= fs->pages_per_sector * fs->sectors) {
      pix = 0;
    }
  } while (pix != pix_end);

  if (v_res == NIFFS_VIS_CONT) {
    res = NIFFS_VIS_END;
  }

  return res;
}

static niffs_file_desc *niffs_get_free_fd(niffs *fs, int *ix) {
  u32_t i;
  for (i = 0; i < fs->descs_len; i++) {
    if (fs->descs[i].obj_id == 0) {
      *ix = i;
      return &fs->descs[i];
    }
  }
  return 0;
}

static void niffs_inform_page_movement(niffs *fs, niffs_page_ix src_pix, niffs_page_ix dst_pix) {
  // update descriptors
  u32_t i;
  for (i = 0; i < fs->descs_len; i++) {
    if (fs->descs[i].obj_id != 0) {
      if (fs->descs[i].obj_pix == src_pix) {
        NIFFS_DBG("inform: pix update (fd%iobj): %04x->%04x oid:%04x\n", i, src_pix, dst_pix, fs->descs[i].obj_id);
        fs->descs[i].obj_pix = dst_pix;
      }
      if (fs->descs[i].cur_pix == src_pix) {
        NIFFS_DBG("inform: pix update (fd%icur): %04x->%04x oid:%04x\n", i, src_pix, dst_pix, fs->descs[i].obj_id);
        fs->descs[i].cur_pix = dst_pix;
      }
    }
  }
}

static void niffs_inform_page_delete(niffs *fs, niffs_page_ix pix) {
  // update descriptors
  u32_t i;
  for (i = 0; i < fs->descs_len; i++) {
    if (fs->descs[i].obj_id != 0) {
      if (fs->descs[i].obj_pix == pix) {
        NIFFS_DBG("  info: pix delete (fd%iobj): %04x oid:%04x\n", i, pix, fs->descs[i].obj_id);
        // someone deleted the object header of this file, close descriptor
        fs->descs[i].obj_pix = 0;
        fs->descs[i].obj_id = 0;
      }
      if (fs->descs[i].cur_pix == pix && fs->descs[i].obj_id > 0) {
        NIFFS_DBG("  info: pix delete (fd%icur): %04x oid:%04x\n", i, pix, fs->descs[i].obj_id);
        fs->descs[i].cur_pix = fs->descs[i].obj_pix;
        fs->descs[i].offs = 0;
      }
    }
  }
}

int niffs_get_filedesc(niffs *fs, int fd_ix, niffs_file_desc **fd) {
  if (fd_ix < 0 || fd_ix >= (int)fs->descs_len) check(ERR_NIFFS_FILEDESC_BAD);
  if (fs->descs[fd_ix].obj_id == 0) check(ERR_NIFFS_FILEDESC_CLOSED);
  *fd = &fs->descs[fd_ix];
  return NIFFS_OK;
}

typedef struct {
  const char *conflict_name;
} niffs_find_free_id_arg;

static int niffs_find_free_id_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)pix;
  niffs_find_free_id_arg *arg = (niffs_find_free_id_arg *)v_arg;
  if (!_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    niffs_obj_id oid = phdr->id.obj_id;
    --oid;
    fs->buf[oid/8] |= 1<<(oid&7);
    if (arg->conflict_name && phdr->id.spix == 0) {
      // object header page
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      if (strcmp(arg->conflict_name, (char *)ohdr->name) == 0) {
        check(ERR_NIFFS_NAME_CONFLICT);
      }
    }
  }
  return NIFFS_VIS_CONT;
}

TESTATIC int niffs_find_free_id(niffs *fs, niffs_obj_id *oid, const char *conflict_name) {
  if (oid == 0) check(ERR_NIFFS_NULL_PTR);
  niffs_memset(fs->buf, 0, fs->buf_len);
  niffs_find_free_id_arg arg = {.conflict_name = conflict_name};
  int res = niffs_traverse(fs, 0, 0, niffs_find_free_id_v, &arg);

  if (res != NIFFS_VIS_END) check(res);

  u32_t max_id = (fs->pages_per_sector * fs->sectors) - 2;
  u32_t cur_id;
  for (cur_id = 0; cur_id < max_id; cur_id += 8) {
    if (fs->buf[cur_id/8] == 0xff) continue;
    u8_t bit_ix;
    for (bit_ix = 0; bit_ix < 8; bit_ix++) {
      if ((fs->buf[cur_id/8] & (1<<bit_ix)) == 0 && (cur_id + bit_ix) + 1 < max_id) {
        *oid = (cur_id + bit_ix) + 1;
        return NIFFS_OK;
      }
    }
  }

  res = ERR_NIFFS_NO_FREE_ID;
  check(res);
  return res;
}

typedef struct {
  niffs_page_ix *pix;
  u32_t excl_sector;
} niffs_find_free_page_arg;

static int niffs_find_free_page_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  niffs_find_free_page_arg *arg = (niffs_find_free_page_arg *)v_arg;
  if (arg->excl_sector != NIFFS_EXCL_SECT_NONE && _NIFFS_PIX_2_SECTOR(fs, pix) == arg->excl_sector) {
    return NIFFS_VIS_CONT;
  }
  if (_NIFFS_IS_FREE(phdr) && _NIFFS_IS_CLEA(phdr)) {
    *arg->pix = pix;
    return NIFFS_OK;
  }
  return NIFFS_VIS_CONT;
}

TESTATIC int niffs_find_free_page(niffs *fs, niffs_page_ix *pix, u32_t excl_sector) {
  if (pix == 0) check(ERR_NIFFS_NULL_PTR);

  niffs_find_free_page_arg arg = {
      .pix = pix,
      .excl_sector = excl_sector
  };
  int res = niffs_traverse(fs, fs->last_free_pix, fs->last_free_pix, niffs_find_free_page_v, &arg);
  if (res == NIFFS_VIS_END) {
    res = ERR_NIFFS_NO_FREE_PAGE;
  } else {
    fs->last_free_pix = *pix;
  }
  return res;
}

typedef struct {
  niffs_page_ix pix;
  u8_t mov_found;
  niffs_page_ix pix_mov;
  niffs_obj_id oid;
  niffs_span_ix spix;
} niffs_find_page_arg;

static int niffs_find_page_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  niffs_find_page_arg *arg = (niffs_find_page_arg *)v_arg;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr) &&
      phdr->id.obj_id == arg->oid && phdr->id.spix == arg->spix) {
    if (arg->mov_found) {
      // had a previous moving page - delete this
      int res = niffs_delete_page(fs, arg->pix_mov);
      check(res);
      arg->mov_found = 0;
    }

    if (_NIFFS_IS_MOVI(phdr)) {
      arg->mov_found = 1;
      arg->pix_mov = pix;
      return NIFFS_VIS_CONT;
    } else {
      arg->pix = pix;
      return NIFFS_OK;
    }
  }
  return NIFFS_VIS_CONT;
}

TESTATIC int niffs_find_page(niffs *fs, niffs_page_ix *pix, niffs_obj_id oid, niffs_span_ix spix, niffs_page_ix start_pix) {
  if (pix == 0) check(ERR_NIFFS_NULL_PTR);

  niffs_find_page_arg arg = {
    .oid = oid,
    .spix = spix,
    .mov_found = 0
  };
  int res = niffs_traverse(fs, start_pix, start_pix, niffs_find_page_v, &arg);
  if (res == NIFFS_VIS_END) {
    if (arg.mov_found) {
      NIFFS_DBG("  find: pix %04x warn found MOVI when looking for obj id:%04x spix:%i\n", arg.pix_mov, oid, spix);
      *pix = arg.pix_mov;
      res = NIFFS_OK;
    } else {
      res = ERR_NIFFS_PAGE_NOT_FOUND;
    }
  } else {
    *pix = arg.pix;
  }
  return res;
}

TESTATIC int niffs_erase_sector(niffs *fs, u32_t sector_ix) {
  niffs_sector_hdr shdr;
  niffs_sector_hdr *target_shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, sector_ix);
  if (target_shdr->abra == _NIFFS_SECT_MAGIC(fs)) {
    // got magic, presume valid erase count
    shdr.era_cnt = target_shdr->era_cnt+1;

    if (shdr.era_cnt < target_shdr->era_cnt && fs->max_era > ERA_CNT_MAX_OF_LIMIT) {
      // max_era = 0xf & era_cnt = 0x0 (era_cnt just wrapped)
      fs->max_era = shdr.era_cnt;
    } else {
      if (fs->max_era < ERA_CNT_MIN_OF_LIMIT && shdr.era_cnt > ERA_CNT_MAX_OF_LIMIT) {
        // e.g. max_era = 0x2 & era_cnt = 0xe
        // pass, max_era less but wrapped
      } else {
        // normal case
        fs->max_era = NIFFS_MAX(shdr.era_cnt, fs->max_era);
      }
    }
  } else {
    // no magic, presume invalid erase count
    shdr.era_cnt = fs->max_era;
  }
  shdr.abra = _NIFFS_SECT_MAGIC(fs);
  NIFFS_DBG("erase : sector %i era_cnt:%i\n", sector_ix, shdr.era_cnt);

  int res = fs->hal_er(_NIFFS_SECTOR_2_ADDR(fs, sector_ix), fs->sector_size);
  if (res == NIFFS_OK) {
    res = fs->hal_wr((u8_t *)_NIFFS_SECTOR_2_ADDR(fs, sector_ix), (u8_t *)&shdr, sizeof(niffs_sector_hdr));
    check(res);
  }
  return res;
}

TESTATIC int niffs_delete_page(niffs *fs, niffs_page_ix pix) {
  niffs_page_id_raw delete_raw_id = _NIFFS_PAGE_DELE_ID;

  niffs_page_hdr *phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
  if (_NIFFS_IS_FREE(phdr)) {
    check(ERR_NIFFS_DELETING_FREE_PAGE);
  }
  if (_NIFFS_IS_DELE(phdr)) check(ERR_NIFFS_DELETING_DELETED_PAGE);
  NIFFS_DBG("  dele: pix %04x\n", pix);
  int res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, pix) + offsetof(niffs_page_hdr, id), (u8_t *)&delete_raw_id, sizeof(niffs_page_id_raw));
  check(res);
  if (res == NIFFS_OK) {
    fs->dele_pages++;
    niffs_inform_page_delete(fs, pix);
  }
  return res;
}

TESTATIC int niffs_move_page(niffs *fs, niffs_page_ix src_pix, niffs_page_ix dst_pix, const u8_t *data, u32_t len, niffs_flag force_flag) {
  if (src_pix == dst_pix) check(ERR_NIFFS_MOVING_TO_SAME_PAGE);

  niffs_page_hdr *src_phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, src_pix);
  niffs_page_hdr *dst_phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, dst_pix);

  int res = NIFFS_OK;
  if (!_NIFFS_IS_FLAG_VALID(src_phdr) || !_NIFFS_IS_FLAG_VALID(dst_phdr)) res = ERR_NIFFS_MOVING_BAD_FLAG;
  else if (_NIFFS_IS_FREE(src_phdr))  res = ERR_NIFFS_MOVING_FREE_PAGE;
  else if (_NIFFS_IS_DELE(src_phdr))  res = ERR_NIFFS_MOVING_DELETED_PAGE;
  else if (!_NIFFS_IS_FREE(dst_phdr)) res = ERR_NIFFS_MOVING_TO_UNFREE_PAGE;
  check(res);

  u8_t src_clear = _NIFFS_IS_CLEA(src_phdr);
  u8_t src_movi = _NIFFS_IS_MOVI(src_phdr);

  NIFFS_DBG("  move: pix %04x->%04x flag:%s\n", src_pix, dst_pix, _NIFFS_IS_CLEA(src_phdr) ? ("CLEA") :
      (_NIFFS_IS_MOVI(src_phdr) ? "MOVI" : "WRIT"));

  // mark src as moving
  if (!_NIFFS_IS_MOVI(src_phdr)) {
    niffs_flag moving_flag = _NIFFS_FLAG_MOVING;
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, src_pix) + offsetof(niffs_page_hdr, flag), (u8_t *)&moving_flag, sizeof(niffs_flag));
    check(res);
  }

  // write dst..
  // .. write flag ..
  niffs_flag flag = _NIFFS_FLAG_CLEAN;
  if (force_flag == NIFFS_FLAG_MOVE_KEEP) {
    if (!src_clear) {
      flag = src_movi ? _NIFFS_FLAG_MOVING : _NIFFS_FLAG_WRITTEN;
    }
  } else {
    flag = force_flag;
  }
  if (flag != _NIFFS_FLAG_CLEAN) {
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, dst_pix) + offsetof(niffs_page_hdr, flag), (u8_t *)&flag, sizeof(niffs_flag));
    check(res);
  }

  fs->free_pages--;
  if (data == 0 && (!src_clear || src_phdr->id.spix == 0)) {
    // .. page data ..
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, dst_pix) + sizeof(niffs_page_hdr), (u8_t *)src_phdr  + sizeof(niffs_page_hdr), fs->page_size - sizeof(niffs_page_hdr));
    check(res);
  } else if (data) {
    // .. else, user data ..
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, dst_pix) + sizeof(niffs_page_hdr), data, len);
    check(res);
  }
  // .. and id
  res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, dst_pix) + offsetof(niffs_page_hdr, id),
      (u8_t *)src_phdr  + offsetof(niffs_page_hdr, id), sizeof(niffs_page_hdr_id));
  check(res);

  niffs_inform_page_movement(fs, src_pix, dst_pix);

  // delete src
  res = niffs_delete_page(fs, src_pix);
  check(res);

  return res;
}

TESTATIC int niffs_write_page(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, const u8_t *data, u32_t len) {
  niffs_page_hdr *orig_phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);

  int res = NIFFS_OK;
  if (phdr->id.obj_id == 0 || phdr->id.obj_id == (niffs_obj_id)-1) {
    res = ERR_NIFFS_WR_PHDR_BAD_ID;
  } else if (!_NIFFS_IS_FREE(orig_phdr) || !_NIFFS_IS_CLEA(orig_phdr)) {
    res = ERR_NIFFS_WR_PHDR_UNFREE_PAGE;
  }
  check(res);

  NIFFS_ASSERT(data == 0 || len <= _NIFFS_SPIX_2_PDATA_LEN(fs, 1));

  NIFFS_DBG("  writ: pix %04x %s oid:%04x spix:%i\n", pix, data ? "DATA":"NODATA", phdr->id.obj_id, phdr->id.spix);

  if (!_NIFFS_IS_CLEA(phdr)) {
    // first, write flag
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, pix) + offsetof(niffs_page_hdr, flag), (u8_t *)&phdr->flag, sizeof(niffs_flag));
    check(res);
  }
  // .. data ..
  if (data) {
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, pix) + sizeof(niffs_page_hdr), data, len);
    check(res);
  }

  // .. then id
  res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, pix) + offsetof(niffs_page_hdr, id), (u8_t *)&phdr->id, sizeof(niffs_page_hdr_id));
  check(res);

  return res;
}

#ifdef NIFFS_TEST
TESTATIC int niffs_write_phdr(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr) {
  return niffs_write_page(fs, pix, phdr, 0, 0);
}
#endif

//////////////////////////////// LINEAR FILES ////////////////////////////////

#if NIFFS_LINEAR_AREA

static int niffs_linear_find_space_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)v_arg;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    if (_NIFFS_IS_OBJ_HDR(phdr)) {
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      if (ohdr->type == _NIFFS_FTYPE_LINFILE) {
        // check linear files only
        // figure out how many sectors this linear file occupy
        niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)phdr;
        u32_t file_len = lfhdr->ohdr.len == NIFFS_UNDEF_LEN ? 0 : lfhdr->ohdr.len;
        u32_t resv_sects = lfhdr->resv_sectors;
        u32_t file_sects = (file_len + fs->sector_size - 1) / fs->sector_size;
        u32_t sects = NIFFS_MAX(resv_sects, file_sects);
        sects = NIFFS_MAX(1, sects);
        if (sects > fs->lin_sectors) {
          // length oob, do not let this file contaminate the free sector map
          // delete this file silently
          (void)niffs_delete_page(fs, pix);
          NIFFS_DBG("   map: linear: pix %04x oid:%04x name:%s bad length %i sectors, deleting\n",
              pix, phdr->id.obj_id, ohdr->name, sects);
          return NIFFS_VIS_CONT;
        }
        u32_t lsix = lfhdr->start_sector - fs->sectors;
        u32_t end_lsix = lsix + sects;
        NIFFS_DBG("   map: linear: oid:%04x name:%s occupies sectors %i--%i\n",
            phdr->id.obj_id, ohdr->name, lsix+fs->sectors, end_lsix+fs->sectors);
        while (lsix < end_lsix) {
          fs->buf[lsix/8] |= (1 << (lsix&7));
          lsix++;
        }
      }
    }
  }
  return NIFFS_VIS_CONT;
}

int niffs_linear_map(niffs *fs) {
  niffs_memset(fs->buf, 0x00, fs->buf_len);
  int res = niffs_traverse(fs, 0, 0, niffs_linear_find_space_v, 0);
  if (res == NIFFS_VIS_END) res = NIFFS_OK;
  check(res);
  return res;
}

int niffs_linear_find_space(niffs *fs, u32_t sectors, u32_t *start_sector) {
  int res = niffs_linear_map(fs);
  check(res);

  // allocate on first fit basis
  u8_t taken = 1;
  u32_t free_sect_start = -1;
  u32_t free_sect_range = 0;
  u32_t lsix;
  for (lsix = 0; lsix < fs->lin_sectors; lsix++) {
    if ((fs->buf[lsix/8] & (1<<(lsix&7))) == 0) {
      // found a free sector
      if (taken) {
        taken = 0;
        free_sect_start = lsix;
        free_sect_range = 1;
      } else {
        free_sect_range++;
      }
      if (free_sect_range >= sectors) {
        // found a range being ok
        break;
      }
    } else {
      // found a full sector
      taken = 1;
    }
  } // per free sector map bit

  if (free_sect_range < sectors || free_sect_start > fs->sectors + fs->lin_sectors) {
    NIFFS_DBG("create: linear: %i free sector range not found\n", sectors);
    check(ERR_NIFFS_LINEAR_NO_SPACE);
  } else {
    *start_sector = free_sect_start + fs->sectors;
    NIFFS_DBG("create: linear: %i free sector range found @ sector %i\n", sectors, *start_sector);
  }

  return res;
}

typedef struct {
  u32_t start_sector;
  u32_t nearest_sector_after;
} niffs_linear_avail_size_arg;

static int niffs_linear_avail_size_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)fs;
  (void)pix;
  niffs_linear_avail_size_arg *arg = (niffs_linear_avail_size_arg *)v_arg;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    if (_NIFFS_IS_OBJ_HDR(phdr)) {
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      if (ohdr->type == _NIFFS_FTYPE_LINFILE) {
        // check linear files only
        niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)phdr;
        if (lfhdr->start_sector > arg->start_sector &&
            lfhdr->start_sector < arg->nearest_sector_after) {
          arg->nearest_sector_after = lfhdr->start_sector;
        }
      }
    }
  }
  return NIFFS_VIS_CONT;
}

int niffs_linear_avail_size(niffs *fs, int fd_ix, u32_t *available_sectors) {
  niffs_file_desc *fd;
  int res = niffs_get_filedesc(fs, fd_ix, &fd);
  check(res);
  if (fd->type != _NIFFS_FTYPE_LINFILE) check(ERR_NIFFS_FILEDESC_BAD);
  niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  if (_NIFFS_IS_DELE(&lfhdr->ohdr.phdr)) res = ERR_NIFFS_PAGE_DELETED;
  else if (_NIFFS_IS_FREE(&lfhdr->ohdr.phdr)) res = ERR_NIFFS_PAGE_FREE;
  else if (lfhdr->ohdr.phdr.id.obj_id != fd->obj_id) res = ERR_NIFFS_INCOHERENT_ID;
  check(res);
  niffs_linear_avail_size_arg arg =
    {.start_sector = lfhdr->start_sector, .nearest_sector_after = (u32_t)-1};
  res = niffs_traverse(fs, 0, 0, niffs_linear_avail_size_v, &arg);
  if (res != NIFFS_VIS_END) return res;
  res = NIFFS_OK;
  if (arg.nearest_sector_after == (u32_t)-1) {
    // no nearest sector so rest is clean we guess
    *available_sectors = fs->lin_sectors + fs->sectors - lfhdr->start_sector;
  } else {
    *available_sectors = arg.nearest_sector_after - lfhdr->start_sector;
  }
  return res;
}

static int niffs_linear_check_erased(niffs *fs, u32_t sector) {
  // assume sectors always are on a 32 bit boundary
  u8_t *a8 = _NIFFS_SECTOR_2_ADDR(fs, sector);
  u32_t *a32 = (u32_t *)a8;
  u32_t slen = fs->sector_size;
  while (slen >= 4) {
    if (*a32 != 0xffffffff) return 1;
    a32++;
    slen -= 4;
  }
  return 0;
}

#endif // NIFFS_LINEAR_AREA

/////////////////////////////////// FILE /////////////////////////////////////

int niffs_create(niffs *fs, const char *name, niffs_file_type type, void *meta) {
  niffs_obj_id oid = 0;
  niffs_page_ix pix;
  int res;

  if (name == 0) check(ERR_NIFFS_NULL_PTR);

  res = niffs_ensure_free_pages(fs, 1);
  check(res);

  res = niffs_find_free_id(fs, &oid, name);
  check(res);

  res = niffs_find_free_page(fs, &pix, NIFFS_EXCL_SECT_NONE);
  check(res);

  NIFFS_DBG("create: pix %04x oid:%04x name:%s\n", pix, oid, name);

  // write page header & clean object header
  niffs_super_hdr hdr;
  hdr.ohdr.phdr.flag = _NIFFS_FLAG_CLEAN;
  hdr.ohdr.phdr.id.obj_id = oid;
  hdr.ohdr.phdr.id.spix = 0;
  hdr.ohdr.len = NIFFS_UNDEF_LEN;
  hdr.ohdr.type = type;
  niffs_strncpy((char *)hdr.ohdr.name, name, NIFFS_NAME_LEN);

  u32_t xtra_meta_len = 0;
  switch (type) {
  case _NIFFS_FTYPE_FILE:
    xtra_meta_len = sizeof(niffs_object_hdr) - sizeof(niffs_page_hdr);
    break;
  case _NIFFS_FTYPE_LINFILE: {
#if NIFFS_LINEAR_AREA
    niffs_linear_file_hdr local_lfhdr;
    niffs_linear_file_hdr *lfhdr;
    if (meta == 0) {
      u32_t lsix_start;
      res = niffs_linear_find_space(fs, 1, &lsix_start);
      if (res < 0) check(res);
      local_lfhdr.start_sector = lsix_start;
      local_lfhdr.resv_sectors = 1;
      lfhdr = &local_lfhdr;
    } else {
      lfhdr = (niffs_linear_file_hdr *)meta;
    }
    niffs_memcpy(
        (u8_t *)&hdr + sizeof(niffs_object_hdr),
        (u8_t *)lfhdr + sizeof(niffs_object_hdr),
        sizeof(niffs_linear_file_hdr) - sizeof(niffs_object_hdr));
    xtra_meta_len = sizeof(niffs_linear_file_hdr) - sizeof(niffs_page_hdr);
    break;
#else
    (void)meta;
    check(ERR_NIFFS_BAD_CONF);
#endif
  }
  default:
    NIFFS_ASSERT(0);
    check(ERR_NIFFS_BAD_CONF);
  }

  NIFFS_DBG("        type:%02x xtra_meta_len:%i\n", type, xtra_meta_len);
  res = niffs_write_page(fs, pix, &hdr.ohdr.phdr,
      (u8_t *)&hdr.ohdr + offsetof(niffs_object_hdr, len),
      xtra_meta_len);

  check(res);
  fs->free_pages--;

  return res;
}

typedef struct {
  const char *name;
  niffs_page_ix pix;
  niffs_obj_id oid;
  niffs_page_ix pix_mov;
  niffs_obj_id oid_mov;
  niffs_file_type type;
} niffs_open_arg;

static int niffs_open_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)pix;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    if (_NIFFS_IS_OBJ_HDR(phdr)) {
      // object header page
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      niffs_open_arg *arg = (niffs_open_arg *)v_arg;
      if (strcmp(arg->name, (char *)ohdr->name) == 0 && ohdr->len != 0) {
        // found matching name
        if (arg->oid_mov) {
          // had a previous moving page - delete this
          int res = niffs_delete_page(fs, arg->pix_mov);
          check(res);
          arg->oid_mov = 0;
        }
        arg->type = ohdr->type;
        if (_NIFFS_IS_MOVI(phdr)) {
          arg->oid_mov = ohdr->phdr.id.obj_id;
          arg->pix_mov = pix;
          return NIFFS_VIS_CONT;
        } else {
          arg->oid = ohdr->phdr.id.obj_id;
          arg->pix = pix;
          return NIFFS_OK;
        }
      }
    }
  }
  return NIFFS_VIS_CONT;
}

int niffs_open(niffs *fs, const char *name, niffs_fd_flags flags) {
  int fd_ix;
  int res = NIFFS_OK;

  if (name == 0) check(ERR_NIFFS_NULL_PTR);

  niffs_file_desc *fd = niffs_get_free_fd(fs, &fd_ix);
  if (fd == 0) check(ERR_NIFFS_OUT_OF_FILEDESCS);

  niffs_open_arg arg;
  niffs_memset(&arg, 0, sizeof(arg));
  arg.name = name;
  res = niffs_traverse(fs, 0, 0, niffs_open_v, &arg);
  if (res == NIFFS_VIS_END) {
    if (arg.oid_mov != 0) {
      NIFFS_DBG("open  : pix %04x found only movi page\n", arg.pix_mov);
      // tidy up found movi obj hdr page
      niffs_page_ix dst_pix;
      res = niffs_chk_tidy_movi_objhdr_page(fs, arg.pix_mov, &dst_pix);
      check(res);
      arg.oid = arg.oid_mov;
      arg.pix = dst_pix;
    } else {
      check(ERR_NIFFS_FILE_NOT_FOUND);
    }
  } else if (res != NIFFS_OK) {
    return res;
  }
  NIFFS_DBG("open  : \"%s\" found @ pix %04x\n", name, arg.pix);

  niffs_memset(fd, 0, sizeof(niffs_file_desc));
  fd->obj_id = arg.oid;
  fd->obj_pix = arg.pix;
  fd->cur_pix = arg.pix;
  fd->type = arg.type;
  fd->flags = flags;

  check(res);

  return fd_ix;
}

int niffs_close(niffs *fs, int fd_ix) {
  int res = NIFFS_OK;

  if (fd_ix < 0 || fd_ix >= (int)fs->descs_len) check(ERR_NIFFS_FILEDESC_BAD);
  if (fs->descs[fd_ix].obj_id == 0) return NIFFS_OK; // allow closing already closed ones

  niffs_file_desc *fd = &fs->descs[fd_ix];

  niffs_memset(fd, 0, sizeof(niffs_file_desc));

  return res;
}

int niffs_read_ptr(niffs *fs, int fd_ix, u8_t **data, u32_t *avail) {
  niffs_file_desc *fd;
  int res = niffs_get_filedesc(fs, fd_ix, &fd);
  check(res);

  if ((fd->flags & NIFFS_O_RDONLY) == 0) {
    check(ERR_NIFFS_NOT_READABLE);
  }

  niffs_object_hdr *ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  u32_t flen = ohdr->len == NIFFS_UNDEF_LEN ? 0 : ohdr->len;
  if (fd->offs >= flen) {
    *data = 0;
    *avail = 0;
    res =  ERR_NIFFS_END_OF_FILE;
  } else if (_NIFFS_IS_DELE(&ohdr->phdr)) res = ERR_NIFFS_PAGE_DELETED;
  else if (_NIFFS_IS_FREE(&ohdr->phdr)) res = ERR_NIFFS_PAGE_FREE;
  else if (ohdr->phdr.id.obj_id != fd->obj_id) res = ERR_NIFFS_INCOHERENT_ID;
  check(res);

  niffs_page_hdr *phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->cur_pix);
  u32_t rem_tot = flen - fd->offs;
  u32_t rem_page = _NIFFS_SPIX_2_PDATA_LEN(fs, phdr->id.spix) - _NIFFS_OFFS_2_PDATA_OFFS(fs, fd->offs);
  u32_t avail_data;
  if (fd->type == _NIFFS_FTYPE_LINFILE) {
#if !NIFFS_LINEAR_AREA
    check(ERR_NIFFS_BAD_CONF);
#else
    avail_data = rem_tot;
#endif
  } else {
    avail_data = NIFFS_MIN(rem_tot, rem_page);
  }

  if (fd->type == _NIFFS_FTYPE_LINFILE) {
    // linear files
    niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)ohdr;
    u8_t *lin_file_start_addr = _NIFFS_SECTOR_2_ADDR(fs, lfhdr->start_sector);
    *data = lin_file_start_addr + fd->offs;
    *avail = avail_data;
  } else {
    // regular page chopped files
    // make sure span index is coherent
    if (phdr->id.spix != _NIFFS_OFFS_2_SPIX(fs, fd->offs)) {
      niffs_page_ix pix;
      res = niffs_find_page(fs, &pix, fd->obj_id, _NIFFS_OFFS_2_SPIX(fs, fd->offs), fd->cur_pix);
      check(res);
      fd->cur_pix = pix;
      phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->cur_pix);
    }

    if (_NIFFS_IS_DELE(phdr)) res = ERR_NIFFS_PAGE_DELETED;
    else if (_NIFFS_IS_FREE(phdr)) res =  ERR_NIFFS_PAGE_FREE;
    else if (phdr->id.obj_id != fd->obj_id) res = ERR_NIFFS_INCOHERENT_ID;
    check(res);
    *data = (u8_t *)phdr + _NIFFS_OFFS_2_PDATA_OFFS(fs, fd->offs) +
        (phdr->id.spix == 0 ? sizeof(niffs_object_hdr) : sizeof(niffs_page_hdr));
    *avail = avail_data;
  }

  return avail_data;
}

int niffs_seek(niffs *fs, int fd_ix, s32_t offset, u8_t whence) {
  int res = NIFFS_OK;
  niffs_file_desc *fd;

  res = niffs_get_filedesc(fs, fd_ix, &fd);
  check(res);
  niffs_object_hdr *ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  u32_t flen = ohdr->len == NIFFS_UNDEF_LEN ? 0 : ohdr->len;
  s32_t coffs;
  switch (whence) {
  default:
  case NIFFS_SEEK_SET:
    coffs = offset;
    break;
  case NIFFS_SEEK_CUR:
    coffs = fd->offs + offset;
    break;
  case NIFFS_SEEK_END:
    coffs = flen + offset;
    break;
  }

  if (coffs < 0) {
    coffs = 0;
  } else {
    coffs = NIFFS_MIN(flen, (u32_t)coffs);
  }

  if (fd->type != _NIFFS_FTYPE_LINFILE &&
      _NIFFS_OFFS_2_SPIX(fs, (u32_t)coffs) != _NIFFS_OFFS_2_SPIX(fs, fd->offs)) {
    // new page
    if (!((u32_t)coffs == flen && _NIFFS_OFFS_2_PDATA_OFFS(fs, (u32_t)coffs) == 0)) {
      niffs_page_ix seek_pix;
      res = niffs_find_page(fs, &seek_pix, fd->obj_id, _NIFFS_OFFS_2_SPIX(fs, (u32_t)coffs), fd->cur_pix);
      check(res);
      fd->cur_pix = seek_pix;
    }
  }
  fd->offs = (u32_t)coffs;

  return res;
}

int niffs_append(niffs *fs, int fd_ix, const u8_t *src, u32_t len) {
  int res = NIFFS_OK;
  niffs_file_desc *fd;
  res = niffs_get_filedesc(fs, fd_ix, &fd);
  check(res);

  if ((fd->flags & NIFFS_O_WRONLY) == 0) {
    check(ERR_NIFFS_NOT_WRITABLE);
  }

  if (len == 0) return NIFFS_OK;

  u8_t *orig_ohdr_addr = (u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  niffs_object_hdr *orig_ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  niffs_page_ix orig_obj_pix = fd->obj_pix;
  u8_t *dst_ohdr_addr = 0; // used at header update in the end of function
  niffs_page_ix dst_ohdr_pix = 0;

  if (orig_ohdr->phdr.id.obj_id != fd->obj_id) check(ERR_NIFFS_INCOHERENT_ID);

  // CHECK SPACE
  u32_t file_offs = orig_ohdr->len == NIFFS_UNDEF_LEN ? 0 : orig_ohdr->len;
#if NIFFS_LINEAR_AREA
  if (fd->type == _NIFFS_FTYPE_LINFILE) {
    // check space in linear area
    u32_t avail_sects;
    res = niffs_linear_avail_size(fs, fd_ix, &avail_sects);
    check(res);
    avail_sects -= (file_offs + fs->sector_size-1) / fs->sector_size; // minus used sects
    u32_t free_bytes_in_current_sector = fs->sector_size - (file_offs % fs->sector_size);
    if ((file_offs % fs->sector_size) == 0) {
      free_bytes_in_current_sector = 0; // already taken in account for in avail_sects
    }
    NIFFS_DBG("append: linear: fileoffs:%i write %i, avail:%i (avail sects:%i, bytes:%i)\n",
        file_offs,
        len, avail_sects * fs->sector_size + free_bytes_in_current_sector, avail_sects, free_bytes_in_current_sector);
    if (len > avail_sects * fs->sector_size + free_bytes_in_current_sector) {
      check(ERR_NIFFS_LINEAR_NO_SPACE);
    }
    // check space in ordinary area
    if (file_offs == 0) {
      // update from clean header with no file size
      dst_ohdr_addr = orig_ohdr_addr;
      dst_ohdr_pix = fd->obj_pix;
    } else {
      // need one page in ordinary fs area to update object header
      res = niffs_ensure_free_pages(fs, 1);
      check(res);
    }
  }
  else
#endif
  {
    if (file_offs == 0 && _NIFFS_OFFS_2_SPIX(fs, len-1) == 0) {
      // no need to allocate a new page, just fill in existing file
    } else {
      // one extra for new object header
      u32_t needed_pages = _NIFFS_OFFS_2_SPIX(fs, len + file_offs) - _NIFFS_OFFS_2_SPIX(fs, file_offs) +
            (_NIFFS_OFFS_2_PDATA_OFFS(fs, len + file_offs) == 0 ? -1 : 0) +
            (file_offs == 0 ? 0 : 1);
      res = niffs_ensure_free_pages(fs, needed_pages);
      check(res);
    }
  }

  // repopulate if moved by gc
  if (fd->obj_pix != orig_obj_pix) {
    orig_ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
    if (orig_ohdr->phdr.id.obj_id != fd->obj_id) res = ERR_NIFFS_INCOHERENT_ID;
    orig_ohdr_addr = (u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
    orig_obj_pix = fd->obj_pix;
  }
  check(res);

  u32_t data_offs = 0;
  u32_t written = 0;
  if (file_offs > 0 && _NIFFS_IS_WRIT(&orig_ohdr->phdr)) {
    // changing existing file - write flag, mark obj header as MOVI
    niffs_flag flag = _NIFFS_FLAG_MOVING;
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix) + offsetof(niffs_object_hdr, phdr) + offsetof(niffs_page_hdr, flag), (u8_t *)&flag, sizeof(niffs_flag));
    check(res);
  }

  // WRITE DATA
  if (fd->type == _NIFFS_FTYPE_LINFILE) {
#if NIFFS_LINEAR_AREA
    // write atmost one sector per pass
    // if a sector boundary is crossed, check sector if empty - if not, then erase
    niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)orig_ohdr;
    while (res == NIFFS_OK && written < len) {
      u32_t avail;
      if (((file_offs + data_offs) % fs->sector_size) == 0) {
        // on sector boundary
        u32_t lsix = lfhdr->start_sector + (file_offs + data_offs) / fs->sector_size;
        if (niffs_linear_check_erased(fs, lsix)) {
          // not empty, must erase
          NIFFS_DBG("append: linear: erase dirty sector %i\n", lsix);
          res = fs->hal_er(_NIFFS_SECTOR_2_ADDR(fs, lsix), fs->sector_size);
          check(res);
        }
        avail = fs->sector_size;
      } else {
        avail = fs->sector_size - ((file_offs - data_offs) % fs->sector_size);
      }
      avail = NIFFS_MIN(avail, len - written);
      NIFFS_DBG("append: linear: sector %i, obj hdr oid:%04x len:%i\n",
          lfhdr->start_sector + (file_offs + data_offs) / fs->sector_size, fd->obj_id, avail);
      res = fs->hal_wr((u8_t *)_NIFFS_SECTOR_2_ADDR(fs, lfhdr->start_sector) + file_offs + data_offs, src, avail);
      check(res);

      src += avail;
      data_offs += avail;
      written += avail;
      fd->offs += avail;
    }
#else
    check(ERR_NIFFS_LINEAR_FILE);
#endif
  } else {
    // operate on per page basis
    while (res == NIFFS_OK && written < len) {
      u32_t avail;

      // case 1: newly created empty file, fill in object header
      if (file_offs + data_offs == 0) {
        // just fill up obj header
        avail = NIFFS_MIN(len, _NIFFS_SPIX_2_PDATA_LEN(fs, 0));
        NIFFS_DBG("append: pix %04x obj hdr oid:%04x spix:0 len:%i\n", fd->obj_pix, fd->obj_id, avail);
        // .. data ..
        res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix) + sizeof(niffs_object_hdr), src, avail);
        check(res);

        dst_ohdr_addr = (u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix); // original obj hdr
        dst_ohdr_pix = fd->obj_pix;
      }

      // case 2: add a new page
      else if (_NIFFS_OFFS_2_PDATA_OFFS(fs, file_offs + data_offs) == 0) {
        // find new page
        avail = NIFFS_MIN(len - written, _NIFFS_SPIX_2_PDATA_LEN(fs, 1));
        niffs_page_ix new_pix;
        res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
        check(res);

        niffs_page_hdr new_phdr;
        new_phdr.id.obj_id = fd->obj_id;
        new_phdr.id.spix = _NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs);
        new_phdr.flag = _NIFFS_FLAG_WRITTEN;
        NIFFS_DBG("append: pix %04x full page oid:%04x spix:%i len:%i\n", new_pix, fd->obj_id, new_phdr.id.spix, avail);

        res = niffs_write_page(fs, new_pix, &new_phdr, src, avail);
        check(res);
        fs->free_pages--;
        fd->cur_pix = new_pix;
      }

      // case 3: rewrite a page
      else {
        niffs_page_ix src_pix;
        if (_NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs) == 0) {
          // rewriting object header page
          src_pix = fd->obj_pix;
        } else {
          // rewriting plain data page, so go get it
          res = niffs_find_page(fs, &src_pix, fd->obj_id, _NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs), fd->cur_pix);
          check(res);
        }

        // find new page
        avail = NIFFS_MIN(len - written,
            _NIFFS_SPIX_2_PDATA_LEN(fs, _NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs)) -
            _NIFFS_OFFS_2_PDATA_OFFS(fs, file_offs + data_offs));
        niffs_page_ix new_pix;
        res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
        check(res);
        NIFFS_DBG("append: pix %04x is free\n", new_pix);

        // modify page
        if (_NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs) == 0) {
          NIFFS_DBG("append: pix %04x modify obj hdr oid:%04x spix:%i len:%i\n", fd->obj_pix, fd->obj_id, 0, avail);
          // rewriting object header, include object header data
          // copy header and current data
          _NIFFS_RD(fs, fs->buf, (u8_t *)_NIFFS_PIX_2_ADDR(fs, src_pix), sizeof(niffs_object_hdr) + file_offs);
          // copy from new data
          niffs_memcpy(&fs->buf[sizeof(niffs_object_hdr) + file_offs], src, avail);

          // reset new object header to be written
          niffs_object_hdr *new_ohdr_data = (niffs_object_hdr *)(fs->buf);
          new_ohdr_data->len = NIFFS_UNDEF_LEN;
          new_ohdr_data->phdr.flag = _NIFFS_FLAG_CLEAN;
          dst_ohdr_addr = (u8_t *)_NIFFS_PIX_2_ADDR(fs, new_pix);
          dst_ohdr_pix = new_pix;
          NIFFS_DBG("append: new obj hdr pix %04x\n", new_pix);
          res = niffs_write_page(fs, new_pix, &new_ohdr_data->phdr, &fs->buf[sizeof(niffs_page_hdr)],
              _NIFFS_SPIX_2_PDATA_LEN(fs, 1));
          check(res);
          fs->free_pages--;
        } else {
          // rewrite plain data page
          // copy from src
          _NIFFS_RD(fs, fs->buf,
              (u8_t *)_NIFFS_PIX_2_ADDR(fs, src_pix) +
                (_NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs) == 0 ? sizeof(niffs_object_hdr) : sizeof(niffs_page_hdr)),
              _NIFFS_OFFS_2_PDATA_OFFS(fs, file_offs + data_offs));
          // copy from new data
          niffs_memcpy(&fs->buf[ _NIFFS_OFFS_2_PDATA_OFFS(fs, file_offs + data_offs)],
              src, avail);
          NIFFS_DBG("append: pix %04x modify page oid:%04x spix:%i len:%i\n", src_pix, fd->obj_id, (u32_t)_NIFFS_OFFS_2_SPIX(fs, file_offs + data_offs), avail);
          NIFFS_DBG("append: new pix %04x\n", new_pix);

          // move page, rewrite data
          res = niffs_move_page(fs, src_pix, new_pix, fs->buf, _NIFFS_SPIX_2_PDATA_LEN(fs, 1), _NIFFS_FLAG_WRITTEN);
          check(res);
        }

        fd->cur_pix = new_pix;
      }

      src += avail;
      data_offs += avail;
      written += avail;
      fd->offs += avail;
    }
  }

  check(res);

  // HEADER UPDATE
  if (written == 0) {
    // haven't written anything, no need to update
    // note: if header has been marked as movi, this will be remedied
    // once the file is opened again or on a check
    return res;
  }
  // move original object header if necessary
  if (dst_ohdr_addr == 0) {
    // find free page
    niffs_page_ix new_pix;
    res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
    NIFFS_DBG("append: header update for new object hdr (only length change), pix %04x\n", new_pix);
    check(res);

    dst_ohdr_addr = (u8_t *)_NIFFS_PIX_2_ADDR(fs, new_pix);

    // copy from old hdr
    _NIFFS_RD(fs, fs->buf, orig_ohdr_addr, fs->page_size);

    ((niffs_object_hdr *)fs->buf)->len = len + file_offs;

    // move header page, rewrite length data
    res = niffs_move_page(fs, fd->obj_pix, new_pix, fs->buf + sizeof(niffs_page_hdr), fs->page_size - sizeof(niffs_page_hdr), _NIFFS_FLAG_WRITTEN);
    check(res);
  } else {
    // just fill in clean object header
    // .. write length..
    NIFFS_DBG("append: header update for object hdr (including data), pix %04x\n", dst_ohdr_pix);
    u32_t length = len + file_offs;
    res = fs->hal_wr((u8_t *)dst_ohdr_addr + offsetof(niffs_object_hdr, len), (u8_t *)&length, sizeof(u32_t));
    check(res);
    // .. write flag..
    niffs_flag flag = _NIFFS_FLAG_WRITTEN;
    res = fs->hal_wr((u8_t *)dst_ohdr_addr + offsetof(niffs_object_hdr, phdr) + offsetof(niffs_page_hdr, flag), (u8_t *)&flag, sizeof(niffs_flag));
    check(res);
    // check if object header moved
    if (dst_ohdr_addr != orig_ohdr_addr) {
      NIFFS_DBG("append: header update inform, moved from pix %04x\n", orig_obj_pix);
      niffs_inform_page_movement(fs, orig_obj_pix, dst_ohdr_pix);
      // .. and remove old
      res = niffs_delete_page(fs, orig_obj_pix);
      check(res);
    }
  }

  return res;
}

int niffs_modify(niffs *fs, int fd_ix, u32_t offset, const u8_t *src, u32_t len) {
  int res = NIFFS_OK;
  niffs_file_desc *fd;
  res = niffs_get_filedesc(fs, fd_ix, &fd);
  check(res);

  if ((fd->flags & NIFFS_O_WRONLY) == 0) {
    check(ERR_NIFFS_NOT_WRITABLE);
  }
  if (fd->type == _NIFFS_FTYPE_LINFILE) {
    check(ERR_NIFFS_LINEAR_FILE); // only append and full delete is allowed for linears
  }

  if (len == 0) return NIFFS_OK;
  niffs_object_hdr *orig_ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  niffs_page_ix orig_obj_pix = fd->obj_pix;
  if (orig_ohdr->phdr.id.obj_id != fd->obj_id) check(ERR_NIFFS_INCOHERENT_ID);
  u32_t file_offs = orig_ohdr->len == NIFFS_UNDEF_LEN ? 0 : orig_ohdr->len;
  if (offset + len > file_offs) {
    check(ERR_NIFFS_MODIFY_BEYOND_FILE);
  }

  // CHECK SPACE
  niffs_span_ix start = _NIFFS_OFFS_2_SPIX(fs, offset);
  niffs_span_ix end = _NIFFS_OFFS_2_SPIX(fs, offset+len);

  u32_t needed_pages = end-start+1;
  res = niffs_ensure_free_pages(fs, needed_pages);

  check(res);

  // repopulate if moved by gc
  if (fd->obj_pix != orig_obj_pix) {
    orig_ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
    if (orig_ohdr->phdr.id.obj_id != fd->obj_id) check(ERR_NIFFS_INCOHERENT_ID);
  }

  u32_t written = 0;

  // WRITE DATA
  niffs_page_ix search_pix = fd->obj_pix;
  // operate on per page basis
  while (res == NIFFS_OK && written < len) {
    u32_t avail;
    niffs_span_ix spix = _NIFFS_OFFS_2_SPIX(fs, offset + written);
    u32_t pdata_len = _NIFFS_SPIX_2_PDATA_LEN(fs, spix);
    u32_t pdata_offs = _NIFFS_OFFS_2_PDATA_OFFS(fs, offset + written);
    avail = pdata_len - pdata_offs;
    avail = NIFFS_MIN(len - written, avail);

    u32_t buf_offs = 0;
    if (spix == 0) {
      // copy object hdr data
      niffs_memcpy(fs->buf, (u8_t *)orig_ohdr + sizeof(niffs_page_hdr), sizeof(niffs_object_hdr) - sizeof(niffs_page_hdr));
      buf_offs = sizeof(niffs_object_hdr) - sizeof(niffs_page_hdr);
    }

    // find original page
    niffs_page_ix orig_pix;
    res = niffs_find_page(fs, &orig_pix, fd->obj_id, spix, search_pix);
    check(res);
    search_pix = orig_pix;

    NIFFS_DBG("modify: pix %04x oid:%04x spix:%i offs:%i len:%i\n", orig_pix, fd->obj_id, spix, pdata_offs, avail);

    niffs_page_ix new_pix;

    if (spix == 0 || avail < pdata_len) {
      // in midst of a page
      u8_t *orig_data_addr = _NIFFS_PIX_2_ADDR(fs, orig_pix);
      orig_data_addr += spix == 0 ? sizeof(niffs_object_hdr) : sizeof(niffs_page_hdr);

      if (pdata_offs > 0) {
        // copy original start
        _NIFFS_RD(fs, &fs->buf[buf_offs], orig_data_addr, pdata_offs);
      }

      // copy new data
      niffs_memcpy(&fs->buf[buf_offs + pdata_offs], src, avail);

      if (pdata_offs + avail < pdata_len) {
        // copy original end
        _NIFFS_RD(fs, &fs->buf[buf_offs + pdata_offs + avail], &orig_data_addr[pdata_offs + avail], pdata_len - (pdata_offs + avail));
      }

      // find dst page & move src
      res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
      check(res);
      res = niffs_move_page(fs, orig_pix, new_pix, fs->buf, pdata_len +
          (spix == 0 ? sizeof(niffs_object_hdr) - sizeof(niffs_page_hdr) : 0), _NIFFS_FLAG_WRITTEN);
      check(res);
    } else {
      // a full page
      res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
      check(res);
      res = niffs_move_page(fs, orig_pix, new_pix, src, avail, _NIFFS_FLAG_WRITTEN);
      check(res);
    }

    written += avail;
    src += avail;
    fd->offs += written;
    fd->cur_pix = new_pix;
  }

  return res;
}

typedef struct {
  niffs_obj_id oid;
  niffs_span_ix ge_spix;
} niffs_remove_obj_id_arg;

static int niffs_remove_obj_id_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  if (!_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    niffs_remove_obj_id_arg *arg = (niffs_remove_obj_id_arg *)v_arg;
    if (phdr->id.obj_id == arg->oid && phdr->id.spix >= arg->ge_spix) {
      int res = niffs_delete_page(fs, pix);
      if (res != NIFFS_OK) {
        NIFFS_DBG("trunc  warn: pix %04x could not delete when trunc %04x/%04x : %i\n", pix, phdr->id.obj_id, phdr->id.spix, res);
        check(res);
      }
    }
  }
  return NIFFS_VIS_CONT;
}

int niffs_truncate(niffs *fs, int fd_ix, u32_t new_len) {
  int res = NIFFS_OK;

  niffs_file_desc *fd;
  res = niffs_get_filedesc(fs, fd_ix, &fd);
  check(res);

  if ((fd->flags & NIFFS_O_WRONLY) == 0) {
    check(ERR_NIFFS_NOT_WRITABLE);
  }
  if (fd->type == _NIFFS_FTYPE_LINFILE && new_len != 0) {
    check(ERR_NIFFS_LINEAR_FILE); // only append and full delete is allowed for linears
  }

  niffs_page_ix orig_ohdr_pix = fd->obj_pix;
  niffs_object_hdr *orig_ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
  u32_t flen = orig_ohdr->len == NIFFS_UNDEF_LEN ? 0 : orig_ohdr->len;
  if (orig_ohdr->phdr.id.obj_id != fd->obj_id) res = ERR_NIFFS_INCOHERENT_ID;
  else if (new_len > flen) res = ERR_NIFFS_TRUNCATE_BEYOND_FILE;
  check(res);
  if (new_len != 0 && new_len == flen) {
    return NIFFS_OK;
  }

  // CHECK SPACE
  if (new_len == 0) {
    // no need to allocate a new page, just remove the lot
  } else {
    // one extra for new object header
    res = niffs_ensure_free_pages(fs, 1);
    check(res);
  }

  // repopulate if moved by gc
  if (fd->obj_pix != orig_ohdr_pix) {
    orig_ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix);
    if (orig_ohdr->phdr.id.obj_id != fd->obj_id) res = ERR_NIFFS_INCOHERENT_ID;
  }
  check(res);

  NIFFS_DBG("trunc : make oid %04x %i bytes\n", orig_ohdr->phdr.id.obj_id, new_len);

  // MARK HEADER
  if (new_len) {
    // changing existing file - write flag, mark obj header as MOVI
    niffs_flag flag = _NIFFS_FLAG_MOVING;
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix) + offsetof(niffs_object_hdr, phdr) + offsetof(niffs_page_hdr, flag), (u8_t *)&flag, sizeof(niffs_flag));
    check(res);

    // rewrite new object header, new length
    niffs_page_ix new_pix;
    res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
    check(res);

    // copy from old hdr
    _NIFFS_RD(fs, fs->buf, _NIFFS_PIX_2_ADDR(fs, fd->obj_pix), fs->page_size);

    ((niffs_object_hdr *)fs->buf)->len = new_len;

    // move header page, rewrite length data
    res = niffs_move_page(fs, fd->obj_pix, new_pix, fs->buf + sizeof(niffs_page_hdr), fs->page_size - sizeof(niffs_page_hdr), _NIFFS_FLAG_WRITTEN);
    check(res);
  } else {
    // removing, zero length
    if (fd->type ==_NIFFS_FTYPE_LINFILE) {
      // linear files: just erase header, sectors are lazily erased when overwritten
      res = niffs_delete_page(fs, fd->obj_pix);
      check(res);
      return res;
    } else {
      u32_t length = 0;
      res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, fd->obj_pix) + offsetof(niffs_object_hdr, len), (u8_t *)&length, sizeof(u32_t));
      check(res);
    }
  }

  // REMOVE PAGES
  niffs_span_ix del_start_spix = _NIFFS_OFFS_2_SPIX(fs, new_len);

  if (_NIFFS_OFFS_2_PDATA_OFFS(fs, new_len) || del_start_spix == 0) {
    del_start_spix++;
  }

  niffs_remove_obj_id_arg trunc_arg = {
      .oid = fd->obj_id,
      .ge_spix = del_start_spix
  };

  // might seem unnecessary when spix > EOF, but this is a part of cleaning away garbage as well
  res = niffs_traverse(fs, 0, 0, niffs_remove_obj_id_v, &trunc_arg);
  if (res == NIFFS_VIS_END) res = NIFFS_OK;

  if (res == NIFFS_OK && new_len == 0) {
    // remove header
    res = niffs_delete_page(fs, fd->obj_pix);
    check(res);
  }

  return res;
}

int niffs_rename(niffs *fs, const char *old_name, const char *new_name) {
  niffs_page_ix dst_pix;
  niffs_page_ix src_pix;
  int res;

  if (old_name == 0) check(ERR_NIFFS_NULL_PTR);
  if (new_name == 0) check(ERR_NIFFS_NULL_PTR);

  res = niffs_ensure_free_pages(fs, 1);
  check(res);

  res = niffs_find_free_page(fs, &dst_pix, NIFFS_EXCL_SECT_NONE);
  check(res);

  NIFFS_DBG("rename: name:%s->%s\n", old_name, new_name);

  niffs_open_arg arg;

  // find src file
  niffs_memset(&arg, 0, sizeof(arg));
  arg.name = old_name;
  res = niffs_traverse(fs, 0, 0, niffs_open_v, &arg);
  if (res == NIFFS_VIS_END) {
    if (arg.oid_mov != 0) {
      src_pix = arg.pix_mov;
    } else {
      check(ERR_NIFFS_FILE_NOT_FOUND);
    }
  } else if (res != NIFFS_OK) {
    check(res);
  } else {
    src_pix = arg.pix;
  }

  // find dst file
  niffs_memset(&arg, 0, sizeof(arg));
  arg.name = new_name;
  res = niffs_traverse(fs, 0, 0, niffs_open_v, &arg);
  if (res == NIFFS_VIS_END) {
    if (arg.oid_mov == 0) {
      res = NIFFS_OK;
    } else {
      check(ERR_NIFFS_NAME_CONFLICT);
    }
  } else if (res != NIFFS_OK) {
    check(res);
  } else {
    check(ERR_NIFFS_NAME_CONFLICT);
  }

  // modify obj hdr
  niffs_page_hdr *src_phdr_addr = (niffs_page_hdr *) _NIFFS_PIX_2_ADDR(fs, src_pix);
  _NIFFS_RD(fs, fs->buf, (u8_t *)src_phdr_addr, fs->page_size);
  niffs_strncpy((char *)fs->buf + offsetof(niffs_object_hdr, name), new_name, NIFFS_NAME_LEN);

  // move and rewrite
  res = niffs_move_page(fs, src_pix, dst_pix, fs->buf + sizeof(niffs_page_hdr),
           _NIFFS_SPIX_2_PDATA_LEN(fs, 1), NIFFS_FLAG_MOVE_KEEP);
  check(res);
  return res;
}

///////////////////////////////////// GC /////////////////////////////////////

static int niffs_ensure_free_pages(niffs *fs, u32_t pages) {
  int res = NIFFS_OK;
  int run = 1;

  while (fs->free_pages < fs->pages_per_sector) {
    NIFFS_DBG("ensure: run#%i warn fs crammed, free pages:%i, need at least:%i\n", run, fs->free_pages, fs->pages_per_sector);
    // crammed, even the spare sector is dirty
    if (fs->dele_pages < (fs->pages_per_sector - fs->free_pages)) {
      // cannot ensure even the spare sector
      check(ERR_NIFFS_OVERFLOW);
    }
    u32_t freed_pages;
    res = niffs_gc(fs, &freed_pages, 0);
    check(res);
    run++;
  }

  if (pages > (fs->dele_pages + fs->free_pages - fs->pages_per_sector)) {
    // this will never fit without deleting stuff
    check(ERR_NIFFS_FULL);
  }

  // try cleaning away needed pages
//#define NIFFS_GC_DBG
  if (pages > fs->free_pages || fs->free_pages - pages < fs->pages_per_sector) {
#ifdef NIFFS_GC_DBG
    int zero_runs = 0;
    u32_t orig_dele_pages = fs->dele_pages;
    u32_t orig_free_pages = fs->free_pages;
#endif
    while (pages > fs->free_pages || fs->free_pages - pages < fs->pages_per_sector) {
      NIFFS_DBG("ensure: run#%i need %i free, have %i-%i\n", run, pages, fs->free_pages, fs->pages_per_sector);
      u32_t freed_pages;
      // only allow gcing of a full page each fourth run
      res = niffs_gc(fs, &freed_pages, ((run - 1) % 4) == 0);
      check(res);
#ifdef NIFFS_GC_DBG
      if (freed_pages == 0) {
        zero_runs++;
        if (zero_runs > 3) {
          NIFFS_dump(fs);
          printf("want pages: %i   have: %i  had dele: %i  had free: %i\n", pages, fs->free_pages, orig_dele_pages, orig_free_pages);
          NIFFS_ASSERT(0);
        }
      }
#endif
      run++;
    }
  }

  return res;
}

typedef struct {
  u32_t sector;
  niffs_erase_cnt era_cnt;
  u32_t dele_pages;
  u32_t free_pages;
  u32_t busy_pages;
} niffs_gc_sector_cand;

static int niffs_gc_find_candidate_sector(niffs *fs, niffs_gc_sector_cand *cand, u8_t allow_full_sector) {
  u32_t sector;
  u8_t found = 0;

  // find candidate sector
  s32_t cand_score = 0x80000000;
  for (sector = 0; sector < fs->sectors; sector++) {
    niffs_sector_hdr *shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, sector);
    if (shdr->abra != _NIFFS_SECT_MAGIC(fs)) {
      continue;
    }
    niffs_erase_cnt shdr_era_cnt = shdr->era_cnt;

    u32_t p_free = 0;
    u32_t p_dele = 0;
    u32_t p_busy = 0;

    niffs_page_ix ipix;
    for (ipix = 0; ipix < fs->pages_per_sector; ipix++) {
      niffs_page_ix pix = _NIFFS_PIX_AT_SECTOR(fs, sector) + ipix;
      niffs_page_hdr *phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
      if (_NIFFS_IS_FREE(phdr) && _NIFFS_IS_CLEA(phdr)) {
        p_free++;
      } else if (_NIFFS_IS_DELE(phdr) || !_NIFFS_IS_FLAG_VALID(phdr)) {
        p_dele++;
      } else {
        p_busy++;
      }
    }

    niffs_erase_cnt era_cnt_diff_typed = fs->max_era - shdr_era_cnt;
    u32_t era_cnt_diff = (u32_t)era_cnt_diff_typed;

    NIFFS_DBG("    gc: sector %2i era:%6i era_d:%4i free:%2i dele:%2i busy:%2i  -- ", sector, shdr_era_cnt, era_cnt_diff, p_free, p_dele, p_busy);

    // never gc a sector that is totally free
    if (p_free == fs->pages_per_sector) {
      NIFFS_DBG("skipped, totally free\n");
      continue;
    }
    // never select sectors that have no room for movement
    if (p_busy > fs->free_pages) {
      NIFFS_DBG("no room for movement\n");
      continue;
    }
    // never select the sector containing the only free pages we've got
    if (fs->free_pages > 0 && p_free == fs->free_pages) {
      NIFFS_DBG("contains the only free pages\n");
      continue;
    }
    // only gc full sectors if allowed
    if (p_busy == fs->pages_per_sector && !allow_full_sector) {
      NIFFS_DBG("full, not allowed\n");
      continue;
    }

    // nb: this might select a sector being full with busy pages
    //     but having too low an erase count - this will free
    //     zero pages, but will move long-lived files hogging a
    //     full sector which ruins the wear leveling
    s32_t score = NIFFS_GC_SCORE(era_cnt_diff,
        (100*p_free)/fs->pages_per_sector,
        (100*p_dele)/fs->pages_per_sector,
        (100*p_busy)/fs->pages_per_sector);
    NIFFS_DBG("score %i\n", score);
    if (score > cand_score && p_busy <= fs->free_pages) {
      cand_score = score;
      cand->sector = sector;
      cand->era_cnt = shdr_era_cnt;
      cand->free_pages = p_free;
      cand->dele_pages = p_dele;
      cand->busy_pages = p_busy;
      found = 1;
    }
  }

  if (found) {
    NIFFS_DBG("gc    : found candidate sector %i era_cnt:%i (free:%i dele:%i busy:%i)\n", cand->sector, cand->era_cnt, cand->free_pages, cand->dele_pages, cand->busy_pages);
  } else {
    NIFFS_DBG("gc    : found no candidate sector\n");
  }

  int res = found ? NIFFS_OK : ERR_NIFFS_NO_GC_CANDIDATE;
  check(res);
  return res;
}

int niffs_gc(niffs *fs, u32_t *freed_pages, u8_t allow_full_sector) {
  niffs_gc_sector_cand cand;
  int res = niffs_gc_find_candidate_sector(fs, &cand, allow_full_sector);
  check(res);

  // move all busy pages within sector
  niffs_page_ix ipix;
  for (ipix = 0; ipix < fs->pages_per_sector; ipix++) {
    niffs_page_ix pix = _NIFFS_PIX_AT_SECTOR(fs, cand.sector) + ipix;
    niffs_page_hdr *phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
    if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
      niffs_page_ix new_pix;
      // find dst page & move src
      res = niffs_find_free_page(fs, &new_pix, cand.sector);
      check(res);
      res = niffs_move_page(fs, pix, new_pix, 0, 0, NIFFS_FLAG_MOVE_KEEP);
      check(res);
    }
  }

  // erase sector
  res = niffs_erase_sector(fs, cand.sector);
  check(res);

  // move free cursor if necessary
  if (_NIFFS_PIX_2_SECTOR(fs, fs->last_free_pix) == cand.sector) {
    u32_t new_free_s = cand.sector+1;
    if (new_free_s >= fs->sectors) {
      new_free_s = 0;
    }
    fs->last_free_pix = _NIFFS_PIX_AT_SECTOR(fs, new_free_s);
  }

  // update stats
  fs->dele_pages -= cand.dele_pages;
  fs->dele_pages -= cand.busy_pages; // this is added by moving all busy pages in erased sector
  fs->free_pages += (cand.dele_pages + cand.busy_pages);
  *freed_pages = cand.dele_pages;

  NIFFS_DBG("gc    : freed %i pages (%i dele, %i busy)\n", *freed_pages, cand.dele_pages, cand.busy_pages);

  return res;
}

/////////////////////////////////// CHECK ////////////////////////////////////

static int niffs_map_obj_hdr_ids_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)pix;
  (void)v_arg;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    if (phdr->id.spix == 0) {
      // object header page
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      if (ohdr->len != NIFFS_UNDEF_LEN && ohdr->len > 0 && ohdr->type != _NIFFS_FTYPE_LINFILE) {
        // Only mark those having a defined length > 0, this way we will remove all unfinished appends
        // to clean file and unfinished deletions.
        // Linear files are not examined, as corresponding data does not reside amongst pages
        // but in a different area.
        niffs_obj_id oid = phdr->id.obj_id;
        --oid;
        fs->buf[oid/8] |= 1<<(oid&7);
      }
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_chk_delete_orphan_bad_dirty_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)v_arg;
  int res;
  if (!_NIFFS_IS_DELE(phdr) &&
      (!_NIFFS_IS_FLAG_VALID(phdr) ||
      (_NIFFS_IS_FREE(phdr) && (_NIFFS_IS_WRIT(phdr) || _NIFFS_IS_MOVI(phdr))) ||
      (!_NIFFS_IS_FREE(phdr) && _NIFFS_IS_CLEA(phdr)))) {
    // found a page bad flag status
    NIFFS_DBG("check : pix %04x bad flag status fl/id:%04x/%04x delete hard\n", pix, phdr->flag, phdr->id.raw);
    niffs_page_id_raw delete_raw_id = _NIFFS_PAGE_DELE_ID;
    res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, pix) + offsetof(niffs_page_hdr, id), (u8_t *)&delete_raw_id, sizeof(niffs_page_id_raw));
    check(res);
  } else if (!_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    niffs_obj_id oid = phdr->id.obj_id;
    --oid;
    niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
    if (phdr->id.spix > 0 && (fs->buf[oid/8] & 1<<(oid&7)) == 0) {
      // found a page with id not belonging to any object header
      NIFFS_DBG("check : pix %04x orphan by id oid:%04x delete\n", pix, oid+1);
      res = niffs_delete_page(fs, pix);
      check(res);
    } else if (phdr->id.spix == 0 && ohdr->len == 0) {
      // found an object header page with size 0
      NIFFS_DBG("check : pix %04x unfinished remove oid:%04x delete\n", pix, oid+1);
      res = niffs_delete_page(fs, pix);
      check(res);
    } else if (phdr->id.spix == 0 && ohdr->type != _NIFFS_FTYPE_LINFILE &&
        (((sizeof(niffs_span_ix) < 4 &&
            ohdr->len != NIFFS_UNDEF_LEN &&
            ohdr->len > (1 << (8*sizeof(niffs_span_ix))) * fs->page_size)) ||
        ohdr->len > fs->sector_size * (fs->sectors-1))) {
      // found an object header page with crazy size
      NIFFS_DBG("check : pix %04x bad length oid:%04x delete\n", pix, oid+1);
      res = niffs_delete_page(fs, pix);
      check(res);
    }
  } else if (_NIFFS_IS_FREE(phdr) && _NIFFS_IS_CLEA(phdr)) {
    u32_t ix;
    u8_t *addr = (u8_t *)phdr;
    for (ix = 0; ix < fs->page_size; ix++) {
      if (addr[ix] != 0xff) {
        NIFFS_DBG("check : pix %04x free but contains data, delete hard\n", pix);
        niffs_page_id_raw delete_raw_id = _NIFFS_PAGE_DELE_ID;
        res = fs->hal_wr((u8_t *)_NIFFS_PIX_2_ADDR(fs, pix) + offsetof(niffs_page_hdr, id), (u8_t *)&delete_raw_id, sizeof(niffs_page_id_raw));
        check(res);
        break;
      }
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_chk_delete_orphans_by_id_and_bad_flag_and_dirty_pages(niffs *fs) {
  niffs_memset(fs->buf, 0, fs->buf_len);
  // map all ids taken by object headers
  int res = niffs_traverse(fs, 0, 0, niffs_map_obj_hdr_ids_v, 0);
  if (res != NIFFS_VIS_END)  {
    check(res);
    return res;
  }
  // remove orphaned and bad flag pages
  res = niffs_traverse(fs, 0, 0, niffs_chk_delete_orphan_bad_dirty_v, 0);
  if (res != NIFFS_VIS_END)  {
    check(res);
    return res;
  }
  return NIFFS_OK;
}

static int niffs_chk_find_corresponding_nonmovi_page_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)fs;
  (void)pix;
  niffs_page_hdr_id *ref_id = (niffs_page_hdr_id *)v_arg;
  if (!_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr) && _NIFFS_IS_WRIT(phdr)) {
    if (phdr->id.obj_id == ref_id->obj_id && phdr->id.spix  == ref_id->spix) {
      return NIFFS_OK;
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_chk_unfinished_movi_data_pages_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)v_arg;
  int res;
  if (!_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr) && _NIFFS_IS_MOVI(phdr)) {
    if (phdr->id.spix > 0) {
      res = niffs_traverse(fs, pix, pix, niffs_chk_find_corresponding_nonmovi_page_v, &phdr->id);
      if (res == NIFFS_OK) {
        // found written page, delete this
        NIFFS_DBG("check : pix %04x MOVI page has WRIT sibling: delete\n", pix);
        res = niffs_delete_page(fs, pix);
        check(res);
      } else if (res == NIFFS_VIS_END) {
        res = NIFFS_OK;
        // found no written page, update this
        niffs_page_ix new_pix;
        NIFFS_DBG("check : pix %04x MOVI page alone: move to WRIT\n", pix);
        res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
        if (res == ERR_NIFFS_NO_FREE_PAGE) {
          NIFFS_DBG("check : pix %04x MOVI page alone: no free page to move to\n", pix);
          res = NIFFS_OK;
        } else {
          res = niffs_move_page(fs, pix, new_pix, 0, 0, _NIFFS_FLAG_WRITTEN);
        }

        check(res);
      } else {
        // erroneous operation, bail out
        check(res);
        return res;
      }
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_chk_unfinished_movi_data_pages(niffs *fs) {
  int res = niffs_traverse(fs, 0, 0, niffs_chk_unfinished_movi_data_pages_v, 0);
  if (res == NIFFS_VIS_END) {
    res = NIFFS_OK;
  }
  check(res);
  return res;
}

typedef struct {
  u32_t ix;
  u32_t len;
  niffs_page_ix last_pix;
} niffs_chk_movi_objhdr_arg;

typedef struct {
  niffs_obj_id oid;
  niffs_span_ix gt_spix;
} niffs_chk_movi_objhdr_tidy_arg;

static int niffs_chk_movi_objhdr_pages_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  niffs_chk_movi_objhdr_arg *arg = (niffs_chk_movi_objhdr_arg *)v_arg;

  if (!_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr) && _NIFFS_IS_MOVI(phdr)) {
    if (phdr->id.spix == 0) {
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      niffs_page_ix *log = (niffs_page_ix *)fs->buf;
      log[arg->ix] = pix;
      (void)ohdr;
      NIFFS_DBG("  chck: pix %04x register MOVI obj hdr oid:%04x max_spix:%i\n", pix, phdr->id.obj_id, (int)_NIFFS_OFFS_2_SPIX(fs, ohdr->len));
      arg->ix++;
      if (arg->ix >= arg->len) {
        arg->last_pix = pix;
        // log full, report back and handle what we have
        NIFFS_DBG("  chck: pix %04x register MOVI obj hdr log full", pix);
        return NIFFS_VIS_END;
      }
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_chk_movi_objhdr_pages_tidy_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  niffs_chk_movi_objhdr_tidy_arg *t_arg = (niffs_chk_movi_objhdr_tidy_arg *)v_arg;
  int res;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    if (phdr->id.spix > 0 && phdr->id.spix > t_arg->gt_spix && phdr->id.obj_id == t_arg->oid) {
      NIFFS_DBG("  chck: pix %04x found MOVI obj hdr oid:%04x spix:%i delete\n", pix, phdr->id.obj_id, phdr->id.spix);
      res = niffs_delete_page(fs, pix);
      check(res);
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_chk_tidy_movi_objhdr_page(niffs *fs, niffs_page_ix pix, niffs_page_ix *dst_pix) {
  int res;
  niffs_object_hdr *ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
  niffs_chk_movi_objhdr_tidy_arg t_arg = {
      .oid = ohdr->phdr.id.obj_id,
      .gt_spix = _NIFFS_OFFS_2_SPIX(fs, ohdr->len == NIFFS_UNDEF_LEN ? 0 : ohdr->len)
  };
  if (_NIFFS_OFFS_2_PDATA_OFFS(fs, ohdr->len == NIFFS_UNDEF_LEN ? 0 : ohdr->len) == 0) {
    t_arg.gt_spix--;
  }
  if (ohdr->type != _NIFFS_FTYPE_LINFILE) {
    // linear files do not have other pages than object headers in normal area,
    // so this operation will never find anything
    NIFFS_DBG("  chck: find pages oid:%04x spix > %i for deleting\n", t_arg.oid, t_arg.gt_spix);
    res = niffs_traverse(fs, 0, 0, niffs_chk_movi_objhdr_pages_tidy_v, &t_arg);
    if (res == NIFFS_VIS_END) res = NIFFS_OK;
    check(res);
  }

  // move obj hdr as written
  NIFFS_DBG("  chck: pix %04x move as written\n", pix);
  niffs_page_ix new_pix;
  res = niffs_find_free_page(fs, &new_pix, NIFFS_EXCL_SECT_NONE);
  if (res == ERR_NIFFS_NO_FREE_PAGE) {
    NIFFS_DBG("  chck: pix %04x MOVI obj hdr: no free page to move to\n", pix);
    if (dst_pix) *dst_pix = pix;
    res = NIFFS_OK;
  } else {
    if (dst_pix) *dst_pix = new_pix;
#if NIFFS_LINEAR_AREA
    if (ohdr->type == _NIFFS_FTYPE_LINFILE) {
      // linear: check file length, search in last file sector until only ff:s, set length to that
      niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)ohdr;
      u32_t lflen = lfhdr->ohdr.len;
      u32_t lsix = lfhdr->start_sector + lflen / fs->sector_size;
      if (lsix < fs->sectors || lsix > fs->sectors + fs->lin_sectors) {
        // oob, corrupt lfhdr
        NIFFS_DBG("  chck: linear: corrupt - size oob - last sector %i, deleting %04x\n", lsix, pix);
        res = niffs_delete_page(fs, pix);
        check(res);
      } else {
        u8_t *sector = _NIFFS_SECTOR_2_ADDR(fs, lsix);
        u32_t wix;
        u32_t last_non_ff = lflen % fs->sector_size;
        for (wix = last_non_ff; wix < fs->sector_size; wix++) {
          if (sector[wix] != 0xff) last_non_ff = wix;
        }
        u32_t new_len = lflen + (last_non_ff + 1 - (lflen % fs->sector_size));
        niffs_linear_file_hdr new_lfhdr;
        niffs_memcpy(&new_lfhdr, lfhdr, sizeof(niffs_linear_file_hdr));
        new_lfhdr.ohdr.len = new_len;
        NIFFS_DBG("  chck: linear: updating size from %i to %i\n", lflen, new_len);
        res = niffs_move_page(fs, pix, new_pix,
            (u8_t *)&new_lfhdr + sizeof(niffs_page_hdr), sizeof(niffs_linear_file_hdr) - sizeof(niffs_page_hdr),
            _NIFFS_FLAG_WRITTEN);
        check(res);
      }
    }
    else
#endif
    {
      res = niffs_move_page(fs, pix, new_pix, 0, 0, _NIFFS_FLAG_WRITTEN);
      check(res);
    }
  }
  return res;
}

static int niffs_find_duplicate_obj_hdr_ids_v(niffs *fs, niffs_page_ix pix, niffs_page_hdr *phdr, void *v_arg) {
  (void)pix;
  (void)v_arg;
  if (_NIFFS_IS_FLAG_VALID(phdr) && !_NIFFS_IS_FREE(phdr) && !_NIFFS_IS_DELE(phdr)) {
    if (phdr->id.spix == 0) {
      // object header page
      niffs_object_hdr *ohdr = (niffs_object_hdr *)phdr;
      if (ohdr->len != NIFFS_UNDEF_LEN && ohdr->len > 0) {
        // only mark those having a defined length > 0, this way we will remove all unfinished appends
        // to clean file and unfinished deletions
        niffs_obj_id oid = phdr->id.obj_id;
        --oid;
        if (fs->buf[oid/8] & 1<<(oid&7)) {
          // id found before, got duplicate
          NIFFS_DBG("  chck: pix %04x found duplicate obj hdr oid:%04x delete\n", pix, phdr->id.obj_id);
          int res = niffs_delete_page(fs, pix);
          check(res);

        } else {
          // id not found, mark it
          fs->buf[oid/8] |= 1<<(oid&7);
        }
      }
    }
  }
  return NIFFS_VIS_CONT;
}

static int niffs_find_duplicate_obj_hdr_ids(niffs *fs) {
  niffs_memset(fs->buf, 0, fs->buf_len);
  // map all ids taken by object headers, find duplicates
  int res = niffs_traverse(fs, 0, 0, niffs_find_duplicate_obj_hdr_ids_v, 0);
  if (res != NIFFS_VIS_END) {
    check(res);
    return res;
  }
  return NIFFS_OK;
}

static int niffs_chk_movi_objhdr_pages(niffs *fs) {
  // clear buffer
  niffs_memset(fs->buf, 0, fs->buf_len);

  niffs_chk_movi_objhdr_arg arg = {
      .ix = 0,
      .len = fs->buf_len / sizeof(niffs_page_ix),
      .last_pix = 0
  };

  int res;
  do {
    // find a chunk or all movi obj hdrs, and tidy away any orphaned pages
    NIFFS_DBG("check : find MOVI obj pix:%04x--end\n", arg.last_pix);
    res = niffs_traverse(fs, arg.last_pix, 0, niffs_chk_movi_objhdr_pages_v, &arg);
    u8_t cont = res == NIFFS_VIS_END;
    if (res == NIFFS_VIS_END) res = NIFFS_OK;
    check(res);

    // remove all pages with too high a span index for this object header
    NIFFS_DBG("check : find MOVI obj pix:%04x--end - got us %i entries\n", arg.last_pix, arg.ix);
    u32_t i;
    niffs_page_ix *log = (niffs_page_ix *)fs->buf;
    for (i = 0; i < arg.ix; i++) {
      res = niffs_chk_tidy_movi_objhdr_page(fs, log[i], 0);
      check(res);
    }

    if (cont) {
      // either finished, or log full
      if (arg.ix >= arg.len) {
        // log full, continue
        niffs_memset(fs->buf, 0, fs->buf_len);
        arg.ix = 0;
        continue;
      } else {
        // found all movi obj hdrs
        break;
      }
    }
  } while (1);

  return NIFFS_OK;
}

int niffs_chk(niffs *fs) {
  u32_t dummy;
  if (fs->mounted) check(ERR_NIFFS_MOUNTED);

  // niffs_chk will not try to ensure free pages by garbage collection before
  // finding needed free pages. This means that any operation needing a new
  // new page might start eating from the spare sector.
  // However, in aborted cases there can be only one unfinished page (MOV),
  // which might need a new free page. Mostly, pages are deleted during a
  // niffs_chk.

  // fixes aborted sector erases
  int res = niffs_setup(fs);
  check(res);

  // fixes pages with bad headers - aborted in midst of movements
  // fixes object headers with zero length - aborted truncate to zero
  // fixes orphaned data pages, pages with oids having no corresponding object header - safety cleanup
  NIFFS_DBG("check : * delete orphans by id, aborted removes, bad flags, dirty pages\n");
  res = niffs_chk_delete_orphans_by_id_and_bad_flag_and_dirty_pages(fs);
  check(res);

  // fixes data pages marked as moving -
  //     either moves them as written if written page never became finalized
  //     or else deletes moving page if corresponding written page was found
  NIFFS_DBG("check : * finalize moving pages\n");
  res = niffs_chk_unfinished_movi_data_pages(fs);
  check(res);

  // fixes object headers with duplicate ids
  NIFFS_DBG("check : * remove object headers with duplicate ids\n");
  res = niffs_find_duplicate_obj_hdr_ids(fs);
  check(res);

  // fixes orphaned data pages by aborted file length update
  NIFFS_DBG("check : * delete orphans by length\n");
  res = niffs_chk_movi_objhdr_pages(fs);
  check(res);

  // do a gc if crammed
  res = niffs_setup(fs); // recount pages
  check(res);
  if (fs->free_pages < fs->pages_per_sector) {
    NIFFS_DBG("check : * gc needed, %i free, must at least have %i\n", fs->free_pages, fs->pages_per_sector);
    res = niffs_gc(fs, &dummy, 0);
    if (res == ERR_NIFFS_NO_GC_CANDIDATE)  {
      res = ERR_NIFFS_OVERFLOW;
    }
  }

  check(res);

  return res;
}

//////////////////////////////////// SETUP ///////////////////////////////////

static int niffs_setup(niffs *fs) {
  fs->free_pages = 0;
  fs->dele_pages = 0;
  fs->max_era = 0;
  u32_t s;
  u32_t bad_sectors = 0;
  niffs_erase_cnt max_era = 0;
  niffs_erase_cnt min_era = (niffs_erase_cnt)-1;

  for (s = 0; s < fs->sectors; s++) {
    niffs_sector_hdr *shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, s);
    if (shdr->abra != _NIFFS_SECT_MAGIC(fs)) {
      bad_sectors++;
      continue;
    }
    max_era = NIFFS_MAX(shdr->era_cnt, max_era);
    min_era = NIFFS_MIN(shdr->era_cnt, min_era);
  }
  if (min_era < ERA_CNT_MIN_OF_LIMIT && max_era > ERA_CNT_MAX_OF_LIMIT) {
    fs->max_era = min_era;
  } else {
    fs->max_era = max_era;
  }

  // we allow one bad sector only would we lose power during erase of a sector
  if (bad_sectors > 1) {
    check(ERR_NIFFS_NOT_A_FILESYSTEM);
  }

  for (s = 0; s < fs->sectors; s++) {
    niffs_sector_hdr *shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, s);
    if (shdr->abra != _NIFFS_SECT_MAGIC(fs)) {
      NIFFS_DBG("check : erasing uninitialized sector %i\n", s);
      int res = niffs_erase_sector(fs, s);
      check(res);
    }

    niffs_page_ix ipix;
    for (ipix = 0; ipix < fs->pages_per_sector; ipix++) {
      niffs_page_ix pix = _NIFFS_PIX_AT_SECTOR(fs, s) + ipix;
      niffs_page_hdr *phdr = (niffs_page_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
      if (_NIFFS_IS_FREE(phdr)) {
        fs->free_pages++;
      }
      else if (_NIFFS_IS_DELE(phdr) || !_NIFFS_IS_FLAG_VALID(phdr)) {
        fs->dele_pages++;
      }
    }
  }
  return NIFFS_OK;
}

///////////////////////////////////// API ////////////////////////////////////

int NIFFS_init(niffs *fs, u8_t *phys_addr, u32_t sectors, u32_t sector_size, u32_t page_size,
    u8_t *buf, u32_t buf_len, niffs_file_desc *descs, u32_t file_desc_len,
    niffs_hal_erase_f erase_f, niffs_hal_write_f write_f, u32_t lin_sectors) {
  fs->phys_addr = phys_addr;
  fs->sectors = sectors;
  fs->sector_size = sector_size;
  fs->buf = buf;
  fs->buf_len = buf_len;
  fs->hal_er = erase_f;
  fs->hal_wr = write_f;
  fs->descs = descs;
  fs->descs_len = file_desc_len;
  fs->last_free_pix = 0;
  fs->mounted = 0;
  fs->max_era = 0;

  u32_t pages_per_sector = sector_size / page_size;
  niffs_memset(descs, 0, file_desc_len * sizeof(niffs_file_desc));

  // calculate actual page size incl page header - leave room for sector header
  if (sector_size % page_size < sizeof(niffs_sector_hdr)) {
    fs->page_size = page_size
        /* int part */  - (sizeof(niffs_sector_hdr) / pages_per_sector)
        /* frac part */ - ((sizeof(niffs_sector_hdr) % pages_per_sector) != 0 ? 1 : 0);
  } else {
    fs->page_size = page_size;
  }
  fs->page_size = fs->page_size & (~(NIFFS_WORD_ALIGN-1));

  // check conf sanity
  if (fs->page_size == 0 || fs->page_size > fs->sector_size/2) {
    NIFFS_DBG("conf  : page size over/underflow\n");
    check(ERR_NIFFS_BAD_CONF);
  }
  if (sizeof(niffs_page_id_raw)*8 < NIFFS_OBJ_ID_BITS + NIFFS_SPAN_IX_BITS) {
    NIFFS_DBG("conf  : niffs_page_id_raw type too small to fit defines NIFFS_OBJ_ID_BITS and NIFFS_SPAN_IX_BITS\n");
    check(ERR_NIFFS_BAD_CONF);
  }
  if ((((fs->sector_size - sizeof(niffs_sector_hdr)) / fs->page_size) * fs->sectors) > (1<<NIFFS_OBJ_ID_BITS)) {
    NIFFS_DBG("conf  : niffs_obj_id type too small to ensure object id uniqueness of %i pages\n",
        (u32_t)(((fs->sector_size - sizeof(niffs_sector_hdr)) / fs->page_size) * fs->sectors));
    check(ERR_NIFFS_BAD_CONF);
  }
  if (sizeof(niffs_page_ix) < 4 &&
  (((fs->sector_size - sizeof(niffs_sector_hdr)) / fs->page_size) * fs->sectors) > (1<<(sizeof(niffs_page_ix) * 8))) {
    NIFFS_DBG("conf  : niffs_page_ix type too small to address %i pages\n",
        (u32_t)(((fs->sector_size - sizeof(niffs_sector_hdr)) / fs->page_size) * fs->sectors));
    check(ERR_NIFFS_BAD_CONF);
  }
  if (sizeof(niffs_span_ix)*8 < NIFFS_SPAN_IX_BITS) {
    NIFFS_DBG("conf  : niffs_span_ix type too small to fit define NIFFS_SPAN_IX_BITS\n");
    check(ERR_NIFFS_BAD_CONF);
  }
  if (sizeof(niffs_obj_id)*8 < NIFFS_OBJ_ID_BITS) {
    NIFFS_DBG("conf  : niffs_obj_id type too small to fit define NIFFS_OBJ_ID_BITS\n");
    check(ERR_NIFFS_BAD_CONF);
  }
  if (buf_len < page_size || buf_len < (((sector_size * sectors) / page_size)+7) / 8) {
    NIFFS_DBG("conf  : buffer length too small, need %i bytes\n",
        NIFFS_MAX(page_size, (((sector_size * sectors) / page_size)+7) / 8));
    check(ERR_NIFFS_BAD_CONF);
  }

  fs->pages_per_sector = pages_per_sector;

#if NIFFS_LINEAR_AREA
  if (lin_sectors > buf_len*8) {
    // when scanning for free linear space, each bit in workbuffer represents one sector
    NIFFS_DBG("conf  : too many linear sectors, maximum is %i\n", buf_len*8);
    check(ERR_NIFFS_BAD_CONF);
  }
  fs->lin_sectors = lin_sectors;
#else
  (void)lin_sectors;
#endif

  NIFFS_DBG("page size req:         %i\n", page_size);
  NIFFS_DBG("actual page size:      %i\n", fs->page_size);
  NIFFS_DBG("num unique obj ids:    %i\n", (1<<NIFFS_OBJ_ID_BITS)-2);
  NIFFS_DBG("max span ix:           %i\n", (1<<NIFFS_SPAN_IX_BITS));
  NIFFS_DBG("max file data length:  %i\n", (u32_t)((1<<NIFFS_SPAN_IX_BITS) * (fs->page_size - sizeof(niffs_page_hdr)) - sizeof(niffs_object_hdr)));
  NIFFS_DBG("max num files by size: %i\n", (u32_t)(((fs->sector_size - sizeof(niffs_sector_hdr)) / (fs->page_size)) * fs->sectors));

  return NIFFS_OK;
}

int NIFFS_format(niffs *fs) {
  if (fs->mounted) check(ERR_NIFFS_MOUNTED);
  int res = NIFFS_OK;
  u32_t s;
  for (s = 0; res == NIFFS_OK && s < fs->sectors; s++) {
    res = niffs_erase_sector(fs, s);
    check(res);
    niffs_sector_hdr *shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, s);
    if (shdr->abra != _NIFFS_SECT_MAGIC(fs)) {
      NIFFS_DBG("\nfatal: erase sector %i hdr magic, expected %08x, was %08x\n",
          s,
          _NIFFS_SECT_MAGIC(fs),
          ((niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, s))->abra);
      res = ERR_NIFFS_SECTOR_UNFORMATTABLE;
    }
  }
#if NIFFS_LINEAR_AREA
  for (s = fs->sectors; res == NIFFS_OK && s < fs->sectors+fs->lin_sectors; s++) {
    NIFFS_DBG("erase : sector %i linear\n", s);
    res = fs->hal_er(_NIFFS_SECTOR_2_ADDR(fs, s), fs->sector_size);
    check(res);
  }
#endif
  return res;
}

int NIFFS_mount(niffs *fs) {
  if (fs->mounted) check(ERR_NIFFS_MOUNTED);
  int res = niffs_setup(fs);
  check(res);
  fs->mounted = 1;
  return NIFFS_OK;
}

int NIFFS_unmount(niffs *fs) {
  if (!fs->mounted) check(ERR_NIFFS_NOT_MOUNTED);
  u32_t i;
  for (i = 0; i < fs->descs_len; i++) {
    fs->descs[i].obj_id = 0;
  }
  fs->mounted = 0;
  return NIFFS_OK;
}

#ifdef NIFFS_DUMP
void NIFFS_dump(niffs *fs) {
  NIFFS_DUMP_OUT("NIFFS\n");
  NIFFS_DUMP_OUT("sector size : %i\n", fs->sector_size);
  NIFFS_DUMP_OUT("sectors     : %i\n", fs->sectors);
  NIFFS_DUMP_OUT("pages/sector: %i\n", fs->pages_per_sector);
  NIFFS_DUMP_OUT("page size   : %i\n", fs->page_size);
  NIFFS_DUMP_OUT("phys addr   : %p\n", fs->phys_addr);
  NIFFS_DUMP_OUT("free pages  : %i\n", fs->free_pages);
  NIFFS_DUMP_OUT("dele pages  : %i\n", fs->dele_pages);
  u32_t s;
  u32_t tot_free = 0;
  u32_t tot_dele = 0;
  for (s = 0; s < fs->sectors; s++) {
    niffs_sector_hdr *shdr = (niffs_sector_hdr *)_NIFFS_SECTOR_2_ADDR(fs, s);
    NIFFS_DUMP_OUT("sector %2i @ %p  era_cnt:%4i  magic:%s\n", s, shdr, shdr->era_cnt, shdr->abra ==  _NIFFS_SECT_MAGIC(fs) ? "OK" : "BAD");
    niffs_page_ix ipix;
    for (ipix = 0; ipix < fs->pages_per_sector; ipix++) {
      niffs_page_ix pix = _NIFFS_PIX_AT_SECTOR(fs, s) + ipix;
      niffs_object_hdr *ohdr = (niffs_object_hdr *)_NIFFS_PIX_2_ADDR(fs, pix);
      NIFFS_DUMP_OUT("  %04x fl:%04x id:%04x ", pix, ohdr->phdr.flag, ohdr->phdr.id.raw);
      if (_NIFFS_IS_FREE(&ohdr->phdr)) tot_free++;
      if (_NIFFS_IS_DELE(&ohdr->phdr)) tot_dele++;
      NIFFS_DUMP_OUT(_NIFFS_IS_FREE(&ohdr->phdr) ? "FR " : "fr ");
      NIFFS_DUMP_OUT(_NIFFS_IS_DELE(&ohdr->phdr) ? "DE " : "de ");
      NIFFS_DUMP_OUT(_NIFFS_IS_CLEA(&ohdr->phdr) ? "CL " : "cl ");
      NIFFS_DUMP_OUT(_NIFFS_IS_WRIT(&ohdr->phdr) ? "WR " : "wr ");
      NIFFS_DUMP_OUT(_NIFFS_IS_MOVI(&ohdr->phdr) ? "MO " : "mo ");
      NIFFS_DUMP_OUT(_NIFFS_IS_FLAG_VALID(&ohdr->phdr) ? "    " : "BAD ");
      if (!_NIFFS_IS_FREE(&ohdr->phdr) && !_NIFFS_IS_DELE(&ohdr->phdr)) {
        NIFFS_DUMP_OUT("  obj.id:%04x  sp.ix:%02x  ", ohdr->phdr.id.obj_id, ohdr->phdr.id.spix);
        if (ohdr->phdr.id.spix == 0 && _NIFFS_IS_ID_VALID(&ohdr->phdr)) {
          NIFFS_DUMP_OUT("len:%08x  type:%02x", ohdr->len, ohdr->type);
          if (ohdr->type == _NIFFS_FTYPE_LINFILE) {
            niffs_linear_file_hdr *lfhdr = (niffs_linear_file_hdr *)ohdr;
            NIFFS_DUMP_OUT("  start_sec:%d  resv_sec:%d", lfhdr->start_sector, lfhdr->resv_sectors);
          }
          NIFFS_DUMP_OUT("  name:");
          int i;
          for (i = 0; i < NIFFS_NAME_LEN; i++) {
            if (ohdr->name[i] == 0) break;
            NIFFS_DUMP_OUT("%c", ohdr->name[i] < ' ' ? '.' : ohdr->name[i]);
          }
        }
      }
      NIFFS_DUMP_OUT("\n");
    }
  }
  if (tot_dele != fs->dele_pages) {
    NIFFS_DUMP_OUT("FATAL! registered deleted pages:%i, but counted %i\n", fs->dele_pages, tot_dele);
//    NIFFS_ASSERT(0);
  }
  if (tot_free != fs->free_pages) {
    NIFFS_DUMP_OUT("FATAL! registered free pages:%i, but counted %i\n", fs->free_pages, tot_free);
//    NIFFS_ASSERT(0);
  }
}
#endif

