// Common platform functions

#ifndef __COMMON_H__
#define __COMMON_H__

#include "elua_int.h"
#include "lua.h"
#include "platform.h"

// Virtual timers data
// VTMR_FIRST_ID must be LARGER than PLATFORM_TIMER_SYS_ID (as declared in platform.h)
#define VTMR_FIRST_ID           ( 0x200 )
#define VTMR_GET_ID( x )        ( ( x ) - VTMR_FIRST_ID )
#define TIMER_IS_VIRTUAL( x )   ( ( VTMR_NUM_TIMERS > 0 ) && ( ( x ) >= VTMR_FIRST_ID ) && ( ( x ) < VTMR_NUM_TIMERS + VTMR_FIRST_ID ) )

// FS interface
#define CMN_FS_INFO_BEFORE_READDIR      0
#define CMN_FS_INFO_INSIDE_READDIR      1
#define CMN_FS_INFO_AFTER_CLOSEDIR      2
#define CMN_FS_INFO_MEMORY_ERROR        3
#define CMN_FS_INFO_OPENDIR_FAILED      4
#define CMN_FS_INFO_READDIR_FAILED      5
#define CMN_FS_INFO_DIRECTORY_DONE      6

#define CMN_FS_TYPE_DIR                 0
#define CMN_FS_TYPE_FILE                1
#define CMN_FS_TYPE_PATTERN             2
#define CMN_FS_TYPE_ERROR               3
#define CMN_FS_TYPE_FILE_NOT_FOUND      4
#define CMN_FS_TYPE_DIR_NOT_FOUND       5
#define CMN_FS_TYPE_UNKNOWN_NOT_FOUND   6

typedef int ( *p_cmn_fs_walker_cb )( const char*, const struct dm_dirent*, void*, int );

// Functions exported by the common platform layer
void cmn_platform_init(void);
void cmn_virtual_timer_cb(void);
void cmn_int_handler( elua_int_id id, elua_int_resnum resnum );
// Timer-specific functions
int cmn_tmr_int_set_status( elua_int_resnum resnum, int status );
int cmn_tmr_int_get_status( elua_int_resnum resnum );
int cmn_tmr_int_get_flag( elua_int_resnum resnum, int clear );
// System timer generic implemenation
void cmn_systimer_set_base_freq( u32 freq_hz );
void cmn_systimer_set_interrupt_freq( u32 freq_hz );
void cmn_systimer_set_interrupt_period_us( u32 period );
void cmn_systimer_periodic(void);
timer_data_type cmn_systimer_get(void);
// Filesystem-related functions
int cmn_fs_walkdir( const char *path, p_cmn_fs_walker_cb cb, void *pdata, int recursive );
char* cmn_fs_split_path( const char *path, const char **pmask );
int cmn_fs_get_type( const char* path );
char* cmn_fs_path_join( const char *first, ... );
int cmn_fs_is_root_dir( const char *path );
int cmn_fs_check_directory( const char *path );

void cmn_uart_setup_sermux(void);

unsigned int intlog2( unsigned int v );
char lastchar( const char *s );
char firstchar( const char *s );
const char* cmn_str64( u64 x );
void cmn_get_timeout_data( lua_State *L, int pidx, unsigned *pid, timer_data_type *ptimeout );

#endif // #ifndef __COMMON_H__

