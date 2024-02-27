#pragma once

#include <limits.h>

#define SIZEBUF 1024*5
#define SIZENUM 10
#define ERRBUF 2
#define OUTBUF 1
#define INBUF 0

#define GETOPT_HELP_OPTION_DECL \
    "help", no_argument, NULL, GETOPT_HELP_CHAR
    
#define case_GETOPT_HELP_CHAR \
    case GETOPT_HELP_CHAR: \
    help(); \
    break;
    
enum { GETOPT_HELP_CHAR = (CHAR_MIN - 2) };

int numtostr(int num, char *str);
int werror(const char *errtext, int err);
int open_error(const char *progname, const char *name, int err);
int close_error(const char *progname, const char *name, int err);

