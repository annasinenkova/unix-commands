#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "lib.h"

static const char *const progname = "cat: \0";
static const char *const tub = "^I\0";
static const char linefeed = '\n';
static const char space = ' ';
static const char end = '$';
static int num = 0;
static int f;

static struct option const long_options[] = {
    {"number-nonblank", no_argument, NULL, 'b'},
    {"number", no_argument, NULL, 'n'},
    {"squeeze-blank", no_argument, NULL, 's'},
    {"show-ends", no_argument, NULL, 'E'},
    {"show-tabs", no_argument, NULL, 'T'},
    {GETOPT_HELP_OPTION_DECL},
    {NULL, 0, NULL, 0}
};


int
help(void)
{
    const char *text = "\
Usage: <path>cat [OPTION]... [FILE]...\n\
Concatenate FILE(s) to standard output.\n\n\
If the FILE is omitted or set as -, reads standard input.\n\n\
  -b, --number-nonblank    number nonempty output lines, overrides -n\n\
  -E, --show-ends          display $ at end of each line\n\
  -n, --number             number all output lines\n\
  -s, --squeeze-blank      suppress repeated empty output lines\n\
  -T, --show-tabs          display TAB characters as ^I\n\
      --help               show this help and exit\n\n\
Examples:\n\
  ./cat f - g  Output f's contents, then standard input, then g's contents.\n\
  ./cat        Copy standard input to standard output.\n";
    if (write(OUTBUF, text, strlen(text)*sizeof(*text)) == -1) {
        werror("cat: write error, errno = \0", errno);
    }
    exit(0);
}

#define SAFEWRITE(call)\
    flag = call;\
    if (flag == -1) {\
        werror("cat: write error, errno = \0", errno);\
        return 1;\
    }

int
cat_without_options(void)
{
    int n;
    char buf[SIZEBUF];
    int flag = 0;
    while ((n = read(f, buf, SIZEBUF)) > 0) {
        SAFEWRITE(write(OUTBUF, buf, n));
        
    }
    if (n == -1) {
        werror("cat: read error, errno = \0", errno);
        return 1;
    }
    return 0;
}


int
cat(int number_nonblank, int number, int squeeze_blank, int show_ends, int show_tabs)
{
    int n;
    int flag = 0;
    char buf[SIZEBUF];
    char numstr[SIZENUM];
    int prevline = 0;
    int prevfeed = 0;
    while ((n = read(f, buf, SIZEBUF)) > 0) {
        int i = 0;
        int prev = 0;
        while (i < n) {
            if (squeeze_blank) {
                if ((buf[i] == '\n') && (i == n - 1)) {
                    prevfeed = 1;
                }
                else if (prevfeed) {
                    for (; (i < n) && (buf[i] == '\n'); ++i);
                    prevfeed = 0;
                }
                else {
                    for (; (i + 1 < n) && (buf[i] == '\n') && (buf[i + 1] == '\n'); ++i);
                }
                prev = i;
            }
            
            if (number_nonblank && !prevline) {
                if (buf[i] != '\n') {
                    int len = numtostr(++num, numstr);
                    for (int j = 0; j < 6 - len; ++j) {
                        SAFEWRITE(write(OUTBUF, &space, sizeof(space)));
                    }
                    SAFEWRITE(write(OUTBUF, numstr, len*sizeof(*numstr)));
                    for (int j = 0; j < 2; ++j) {
                        SAFEWRITE(write(OUTBUF, &space, sizeof(space)));
                    }
                }
            }
            else if (number && !prevline) {
                int len = numtostr(++num, numstr);
                for (int j = 0; j < 6 - len; ++j) {
                    SAFEWRITE(write(OUTBUF, &space, sizeof(space)));
                }
                SAFEWRITE(write(OUTBUF, numstr, len*sizeof(*numstr)));
                for (int j = 0; j < 2; ++j) {
                    SAFEWRITE(write(OUTBUF, &space, sizeof(space)));
                }
            }
            
            if (show_tabs) {
                while ((i < n) && (buf[i] != '\n')) {
                    if (buf[i] == '\t') {
                        SAFEWRITE(write(OUTBUF, buf + prev, i - prev));
                        SAFEWRITE(write(OUTBUF, tub, 2*sizeof(*tub)));
                        prev = i + 1;
                    }
                    ++i;
                }
                SAFEWRITE(write(OUTBUF, buf + prev, i - prev));
            }
            else {
                for (; (i < n) && (buf[i] != '\n'); ++i);
                SAFEWRITE(write(OUTBUF, buf + prev, i - prev));
            }
            prev = ++i;
            if (i > n) {
                prevline = 1;
                break;   
            }
            if (show_ends) {
                SAFEWRITE(write(OUTBUF, &end, sizeof(end)));
            }
            SAFEWRITE(write(OUTBUF, &linefeed, sizeof(linefeed)));
            prevline = 0;
        }
    }
    if (n == -1) {
        werror("cat: read error, errno = \0", errno);
        return 1;
    }
    return 0;
}


int
main(int argc, char *argv[])
{
    int number_nonblank, number, squeeze_blank, show_ends, show_tabs, opt;
    number_nonblank = number = squeeze_blank = show_ends = show_tabs = 0;
    while ((opt = getopt_long(argc, argv, "bEnsT", long_options, NULL)) != -1) {
        switch (opt) {
            case 'b':
                number_nonblank = 1;
                break;
            case 'E':
                show_ends = 1;
                break;
            case 'n':
                number = 1;
                break;
            case 's':
                squeeze_blank = 1;
                break;
            case 'T':
                show_tabs = 1;
                break;
            case_GETOPT_HELP_CHAR;
            default:
                exit(2);
        }
    }
    int flag = 0;
    int file = optind;
    do {
        if (file >= argc || !strcmp(argv[file], "-")) {
            f = INBUF;
        }
        else if ((f = open(argv[file], O_RDONLY)) == -1) {
            open_error(progname, argv[file], errno);
            ++file;
            continue;
        }
        if (!number_nonblank && !number && !squeeze_blank && !show_ends && !show_tabs) {
            flag = cat_without_options();
        }
        else {
            flag = cat(number_nonblank, number, squeeze_blank, show_ends, show_tabs);
        }
        if (f != INBUF && close(f) == -1) {
            close_error(progname, argv[file], errno);
        }
        if (flag) {
            exit(1);
        }
        ++file;
    } while (file < argc);
    return 0;
}

