// eLua shell 

#ifndef __SHELL_H__
#define __SHELL_H__

#define SHELL_WELCOMEMSG                "\neLua %s  Copyright (C) 2007-2011 www.eluaproject.net\n"
#define SHELL_PROMPT                    "eLua# "
#define SHELL_ERRMSG                    "Invalid command, type 'help' for help\n"
#define SHELL_MAXSIZE                   50
#define SHELL_MAX_LUA_ARGS              8

int shell_init();
void shell_start();
int shellh_cp_file( const char *src, const char *dst, int flags );
void shellh_not_implemented_handler( int argc, char **argv );
void shellh_show_help( const char *cmd, const char *helptext );

#define SHELL_SHOW_HELP( cmd )          shellh_show_help( #cmd, shell_help_##cmd )

// Helpers for various functions
int shellh_ask_yes_no();

// Flags for various operations
#define SHELL_F_RECURSIVE               1
#define SHELL_F_FORCE_DESTINATION       2
#define SHELL_F_ASK_CONFIRMATION        4
#define SHELL_F_SIMULATE_ONLY           8
#define SHELL_F_SILENT                  16
#define SHELL_F_MOVE                    32

#endif // #ifndef __SHELL_H__

