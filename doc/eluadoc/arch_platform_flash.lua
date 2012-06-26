-- eLua platform interface - flash access functions

data_en = 
{
  -- Title
  title = "eLua platform interface - flash access functions",

  -- Menu name
  menu_name = "Flash",

  -- Overview
  overview = [[The flash access section of the platform interface contains functions for writing/erasing the internal flash of the MCU and obtaining information about its internal structure. These functions are used for imlementing WOFS, check @arch_wofs.html@here@ for more information about WOFS.]],




  -- Functions
  funcs = 
  {
    { sig = "u32 #platform_flash_get_first_free_block_address#( u32 *psect );",
      desc = [[Returns the first free address in the internal flash, aligned to a sector boundary. This function relies on a symbol exported by the linker command file, check @arch_wofs.html@here@ for details. This function. This function is implemented in %src/common.c%.]],
      args = "$psect$ - the sector number of the first free sector in flash will be written in $*psect*$ if $psect$ is not NULL.",
      ret = "The first free address in the internal flash.",
    },

    { sig = "u32 #platform_flash_get_sector_of_address#( u32 addr );",
      desc = "Returns the flash sector that contains the given address. This function is implemented in %src/common.c%.",
      arg = "$addr$ - the flash address.",
      ret = "The sector number of the sector that contains $addr$.",
    },

    { sig = "u32 #platform_flash_write#( const void *from, u32 toaddr, u32 size );",
      desc = [[Writes data in the internal flash. This function can automatically take care of flash alignment or size restrictions if $INTERNAL_FLASH_WRITE_UNIT_SIZE$ is properly defined. Check @arch_wofs.html@here@ for more details. This function is implemented in %src/common.c%. In order to actually write data to the internal flash, this function will call its platform specific (@#platform_s_flash_write@platform_s_flash_write@).]],
      args = 
      {
        "$from$ - the data to be written in flash.",
        "$toaddr$ - flash address to write to.",
        "$size$ - length of data in bytes",
      },
      ret = "The actual number of bytes written in flash.",
    },

    { sig = "u32 #platform_s_flash_write#( const void *from, u32 toaddr, u32 size );",
      desc = [[Writes data in the internal flash. This is the platform dependent counterpart of @#platform_flash_write@platform_flash_write@ and it must be implemented by the platform's porting layer. If $INTERNAL_FLASH_WRITE_UNIT_SIZE$ is properly defined, this function doesn't need to care about data alignment or size restriction issues.]],
      args = 
      {
        "$from$ - the data to be written in flash.",
        "$toaddr$ - flash address to write to.",
        "$size$ - length of data in bytes",
      },
      ret = "The actual number of bytes written in flash.",
    },

    {
      sig = "u32 #platform_flash_get_num_sectors#();",
      desc = "Returns the number of sectors in the internal flash. This function is implemented in %src/common.c%.",
      ret = "The number of sectors in the internal flash.",
    },

    {
      sig = "int #platform_flash_erase_sector#( u32 sector_id );",
      desc = [[Erases a sector from the internal flash. The function does not return until the erase operation is completed. This function is implemented in %src/common.c%. <span class="warning">IMPORTANT</span>: be careful when using this function, as it can erase any sector of the flash. You might damage your eLua image if this function is called with a wrong parameter.]],
      args = "$sector_id$ - the number of the sector that will be deleted.",
      ret = "$PLATFORM_OK$ if the erase operation succeeded, $PLATFORM_ERR$ otherwise."
    }
  }
}

