// eLua shell 

#ifndef __SHELL_H__
#define __SHELL_H__

#define SHELL_PROMPT        "eLua# "
#define SHELL_ERRMSG        "Invalid command, type 'help' for help\n"
#define SHELL_MAXSIZE       50
#define SHELL_MAX_LUA_ARGS  8

int shell_init( unsigned maxprog );
void shell_start();

#endif // #ifndef __SHELL_H__
