
// iv-0.39 (modified); An embedded text editor for Hempl.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "shell.h"
#include "common.h"
#include "type.h"
#include "platform_conf.h"
#include "iv.h"

#ifdef BUILD_EDITOR_IV

const char shell_help_iv[] = "<path>\n"
  "  <path>: the file to open.\n"
  "  [-v]: show version information.\n"
  "If <path> is not specified, iv looks for file '/mmc/untitled'.\n"
  "If found, iv opens this for editing. If not found, it creates it.\n";
const char shell_help_summary_iv[] = "Edit file with iv, a vi-like text editor";

void shell_iv( int argc, char **argv )
{
  iv_main( argc, argv );
  clearerr( stdin );
}

#else // #ifdef BUILD_EDITOR_IV

const char shell_help_iv[] = "";
const char shell_help_summary_iv[] = "";

void shell_iv( int argc, char **argv )
{
  shellh_not_implemented_handler( argc, argv );
}

#endif // #ifdef BUILD_EDITOR_IV
