// eLua shell 

#ifndef __SHELL_H__
#define __SHELL_H__

#include "type.h"
#include "platform_conf.h"

#if !defined(SHELL_WELCOMEMSG)
#define SHELL_WELCOMEMSG                "\neLua %s  Copyright (C) 2007-2013 www.eluaproject.net\n"
#endif
#if !defined(SHELL_PROMPT)
#define SHELL_PROMPT                    "eLua# "
#endif
#if !defined(SHELL_ERRMSG)
#define SHELL_ERRMSG                    "Invalid command, type 'help' for help\n"
#endif
#if !defined(SHELL_MAXSIZE)
#define SHELL_MAXSIZE                   50
#endif
#if !defined(SHELL_MAX_LUA_ARGS)
#define SHELL_MAX_LUA_ARGS              8
#endif

// Shell command handler function
typedef void( *p_shell_handler )( int argc, char **argv );

// Command/handler pair structure
typedef struct
{
  const char* cmd;
  p_shell_handler handler_func;
} SHELL_COMMAND;

int shell_init( void );
void shell_start( void );
const SHELL_COMMAND* shellh_execute_command( char* cmd, int interactive_mode );
int shellh_cp_file( const char *src, const char *dst, int flags );
void shellh_not_implemented_handler( int argc, char **argv );
void shellh_show_help( const char *cmd, const char *helptext );

#define SHELL_SHOW_HELP( cmd )          shellh_show_help( #cmd, shell_help_##cmd )

// Helpers for various functions
int shellh_ask_yes_no( const char *prompt );

// Flags for various operations
#define SHELL_F_RECURSIVE               1
#define SHELL_F_FORCE_DESTINATION       2
#define SHELL_F_ASK_CONFIRMATION        4
#define SHELL_F_SIMULATE_ONLY           8
#define SHELL_F_SILENT                  16
#define SHELL_F_MOVE                    32

// External shell function declaration
#define SHELL_FUNC( func )        extern void func( int argc, char **argv )

// Extern implementations of shell functions
SHELL_FUNC( shell_ls );
SHELL_FUNC( shell_cp );
SHELL_FUNC( shell_adv_mv );
SHELL_FUNC( shell_adv_rm );
SHELL_FUNC( shell_recv );
SHELL_FUNC( shell_help );
SHELL_FUNC( shell_cat );
SHELL_FUNC( shell_lua );
SHELL_FUNC( shell_ver );
SHELL_FUNC( shell_mkdir );
SHELL_FUNC( shell_wofmt );

#endif // #ifndef __SHELL_H__

