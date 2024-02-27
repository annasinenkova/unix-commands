#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "lib.h"

static const char *const progname = "cp: \0";

int
help(void)
{
    const char *text = "\
Usage: <path>cp SOURCE DEST\n\
Copy SOURCE to DEST.\n\
    --help    show this help and exit\n";
    if (write(OUTBUF, text, strlen(text)*sizeof(*text)) < 0) {
        werror("cp: write error, errno = \0", errno);
    }
    exit(0);
}

#define SAFEWRITE(call)\
    if (call < 0) {\
        flag = 1;\
    }

int
main(int argc, char *argv[])
{
    int flag = 0;
    if (argc > 1 && !strcmp(argv[1], "--help")) {
        help();
    }
    if (argc != 3) {
        char *text = "cp: no file specified\n\0";
        SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
        exit(1);
    }
    int f_from, f_to, n;
    char buf[SIZEBUF];
    if (!strcmp(argv[1], argv[2])) {
        char *text = "cp: \0";
        SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
        SAFEWRITE(write(ERRBUF, argv[1], strlen(argv[1])*sizeof(*argv[1])));
        text = " and \0";
        SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
        SAFEWRITE(write(ERRBUF, argv[1], strlen(argv[1])*sizeof(*argv[1])));
        text = " - the same file\n\0";
        SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
        exit(1);
    }
    if ((f_from = open(argv[1], O_RDONLY)) == -1) {
        open_error(progname, argv[1], errno);
        exit(2);
    }
    struct stat s;
    stat(argv[1], &s);
    if ((f_to = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, s.st_mode)) == -1) {
        open_error(progname, argv[2], errno);
        if (close(f_from) == -1) {
            close_error(progname, argv[1], errno);
        }
        exit(2);
    }
    while ((n = read(f_from, buf, SIZEBUF)) > 0) {
        if (write(f_to, buf, n) < 0) {
            werror("cp: write error, errno = \0", errno);
            flag = 1;
            break;
        }
    }
    if (n == -1) {
        werror("cp: read error, errno = \0", errno);
        flag = 1;
    }
    if (close(f_from) == -1) {
        close_error(progname, argv[1], errno);
        flag = 1;
    }
    if (close(f_to) == -1) {
        close_error(progname, argv[2], errno);
        flag = 1;
    }
    return flag;
}

