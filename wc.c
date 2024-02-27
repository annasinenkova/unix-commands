#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "lib.h"

static const char *progname = "wc: \0";
static const char linefeed = '\n';
static const char space = ' ';
static int f;

static struct option const long_options[] = {
    {"bytes", no_argument, NULL, 'c'},
    {"chars", no_argument, NULL, 'm'},
    {"lines", no_argument, NULL, 'l'},
    {"words", no_argument, NULL, 'w'},//tub
    {"max-line-length", no_argument, NULL, 'L'},
    {GETOPT_HELP_OPTION_DECL},
    {NULL, 0, NULL, 0}
};

struct parameters {
    int bytes;
    int chars;
    int lines;
    int maxlinelen;
    int words;
    char *name;
};


int
help(void)
{
    const char *text = "\
Usage: <path>wc [OPTION]... [FILE]...\n\
Print newline, word, and byte counts for each FILE, and a total line if\n\
more than one FILE is specified.  A word is a non-zero-length sequence of\n\
characters delimited by white space.\n\n\
If the FILE is omitted or set as -, reads standard input.\n\n\
The options below may be used to select which counts are printed, always in\n\
the following order: newline, word, character, byte, maximum line length.\n\
  -c, --bytes            print the byte counts\n\
  -m, --chars            print the character counts\n\
  -l, --lines            print the newline counts\n\
  -L, --max-line-length  print the maximum display width\n\
  -w, --words            print the word counts\n\
      --help             show this help and exit\n";
    if (write(OUTBUF, text, strlen(text)*sizeof(*text)) < 0) {
        werror("wc: write error, errno = \0", errno);
    }
    exit(0);
}

#define SAFEWRITE(call)\
    if (call < 0) {\
        werror("wc: write error, errno = \0", errno);\
        return 1;\
    }


int
print_res(struct parameters *res, int mlen, int i,
      int p_bytes, int p_chars, int p_lines, int p_maxlinelen, int p_words)
{
    #define PRINT(param) len = numtostr(param, numstr);\
                        for (int j = 0; j < mlen - len; ++j) {\
                            SAFEWRITE(write(OUTBUF, &space, sizeof(space)));\
                        }\
                        SAFEWRITE(write(OUTBUF, numstr, len*sizeof(*numstr)));\
                        SAFEWRITE(write(OUTBUF, &space, sizeof(space)));
    int len;
    char numstr[SIZENUM];
    if (p_lines) {
        PRINT(res[i].lines);
    }   
    if (p_words) {
        PRINT(res[i].words);
    }
    if (p_chars) {
        PRINT(res[i].chars);
    }
    if (p_bytes) {
        PRINT(res[i].bytes);
    }
    if (p_maxlinelen) {
        PRINT(res[i].maxlinelen);
    }
    SAFEWRITE(write(OUTBUF, res[i].name, strlen(res[i].name)*sizeof(*(res[i].name))));
    SAFEWRITE(write(OUTBUF, &linefeed, sizeof(linefeed)));
    return 0;
    #undef PRINT
}


int
wc(struct parameters *res, int size)
{
    int n;
    int len = 0;
    char buf[SIZEBUF];
    res[size].bytes = res[size].chars = res[size].lines = res[size].maxlinelen = res[size].words = 0;
    while ((n = read(f, buf, SIZEBUF)) > 0) {
        res[size].bytes += n;
        res[size].chars += n;
        char *ptr = buf;
        char *end = ptr + n;
        char prev = ' ';
        while (ptr != end) {
            ++len;
            if (*ptr == '\0') {
                break;
            }
            switch (*ptr) {
                case ' ':
                    if (prev != ' ' && prev != '\n') {
                        ++res[size].words;
                    }
                    break;
                //case '\0':
                case '\n':
                    if (--len > res[size].maxlinelen) {
                        res[size].maxlinelen = len;
                    }
                    if (prev != ' ' && prev != '\n') {
                        ++res[size].words;
                    }
                    len = 0;
                    ++res[size].lines;
                    break;
                default:
                    break;
            }
            prev = *ptr++;
        }
    }
    res[0].bytes += res[size].bytes;
    res[0].chars += res[size].chars;
    res[0].lines += res[size].lines;
    res[0].words += res[size].words;
    if (res[size].maxlinelen > res[0].maxlinelen) {
        res[0].maxlinelen = res[size].maxlinelen;
    }
    if (n == -1) {
        werror("wc: read error, errno = \0", errno);
        return 1;
    }
    return 0;
}


int
main(int argc, char *argv[])
{
    char *namebuf = "INBUF\0";
    int p_bytes, p_chars, p_lines, p_files0, p_maxlinelen, p_words, opt;
    p_bytes = p_chars = p_lines = p_files0 = p_maxlinelen = p_words = 0;
    while ((opt = getopt_long(argc, argv, "cmlwL", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                p_bytes = 1;
                break;
            case 'm':
                p_chars = 1;
                break;
            case 'l':
                p_lines = 1;
                break;
            case 'w':
                p_words = 1;
                break;
            case 'L':
                p_maxlinelen = 1;
                break;
            case_GETOPT_HELP_CHAR;
            default:
                exit(2);
        }
    }
    if (!p_bytes && !p_chars && !p_lines && !p_maxlinelen && !p_words) {
        p_lines = p_words = p_bytes = 1;
    }
    struct parameters res[argc - optind + 1 + 5];
    res[0].bytes = res[0].chars = res[0].lines = res[0].maxlinelen = res[0].words = 0;
    res[0].name = "total\0";
    int file = optind;
    int size = 0;
    int flag = 0;
    do {
        if (file >= argc || !strcmp(argv[file], "-")) {
            f = INBUF;
        }
        else if ((f = open(argv[file], O_RDONLY)) == -1) { //open(argv[file], O_RDONLY | O_BINARY)
            open_error(progname, argv[file], errno);
            ++file;
            continue;
        }
        flag = wc(res, ++size);
        if (file >= argc || !strcmp(argv[file], "-")) {
            res[size].name = namebuf;
        }
        else {
            res[size].name = argv[file];
        }
        if (f != INBUF && close(f) == -1) {
            close_error(progname, argv[file], errno);
        }
        if (flag) {
            exit(2);
        }
        ++file;
    } while (file < argc);
    char numstr[SIZENUM];
    int mlen = numtostr(res[0].bytes, numstr);
    for (int i = 1; i <= size; ++i) {
        if (print_res(res, mlen, i, p_bytes, p_chars, p_lines, p_maxlinelen, p_words) == 1) {
            exit(3);
        }
    }
    if (size > 1) {//>2
        if (print_res(res, mlen, 0, p_bytes, p_chars, p_lines, p_maxlinelen, p_words) == 1) {
            exit(3);
        }
    }
    return 0;
}

