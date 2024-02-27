#include <stdio.h>
#include <string.h>
#include <errno.h> 
#include <unistd.h>
#include "lib.h"


static const char linefeed = '\n';
static const char space = ' ';
static const char colon = ':';

int
numtostr(int num, char *str)
{
    sprintf(str, "%d", num);
    int len = strlen(str);
    return len;
}

#define SAFEWRITE(call)\
    if (call < 0) {\
        return 1;\
    }

#define WRITEERRNO\
    char numstr[SIZENUM];\
    int len = numtostr(err, numstr);\
    SAFEWRITE(write(ERRBUF, numstr, len*sizeof(*numstr)));\
    SAFEWRITE(write(ERRBUF, &linefeed, sizeof(linefeed)))

    
int
werror(const char *errtext, int err)
{
    SAFEWRITE(write(ERRBUF, errtext, strlen(errtext)*sizeof(*errtext)));
    WRITEERRNO;
    return 0;
}


int
open_error(const char *progname, const char *name, int err)
{
    SAFEWRITE(write(ERRBUF, progname, strlen(progname)*sizeof(*progname)));
    SAFEWRITE(write(ERRBUF, name, strlen(name)*sizeof(*name))); 
    if (err == ENOENT) {
        char *text = ": there is no such file\n\0";
        SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
    }
    else {
        char *text = ": open error, errno = \0";
        SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
        WRITEERRNO;
    }
    return 0;
}


int
close_error(const char *progname, const char *name, int err)
{
    SAFEWRITE(write(ERRBUF, progname, strlen(progname)*sizeof(*progname)));
    SAFEWRITE(write(ERRBUF, name, strlen(name)*sizeof(*name))); 
    char *text = ": close error, errno = \0";
    SAFEWRITE(write(ERRBUF, text, strlen(text)*sizeof(*text)));
    WRITEERRNO;
    return 0;
}

