#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <unistd.h>

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
#error C99 compliant compilator required
#endif

#if !defined(_POSIX_VERSION) || (_POSIX_VERSION < 200809L)
#error POSIX.1-2008 expected to be supported
#endif

#if !defined(_XOPEN_VERSION) || (_XOPEN_VERSION < 700)
#error Open Group Single UNIX Specification, Version 4 (SUSv4) expected to be supported
#endif

#if !defined(__gnu_linux__)
#error GNU Linux is definitely supported by now
#endif

#include <stdio.h>
#include <string.h>
#include <getopt.h>

/*
 * protect from changing root credentials
 */
int runArgsCheck(char** cmdLine, char** rArgs) {
    int ac = 0;
    for (char** p = cmdLine; *p != NULL; p++) {
        ac++;
    }

    char* opts = ":kdluefx:n:w:i:S?";
    struct option* lopts = (struct option[]) {
        { "keep-tokens", 0, NULL, 'k' },
        { "delete", 0, NULL, 'd' },
        { "lock", 0, NULL, 'l' },
        { "unlock", 0, NULL, 'u' },
        { "expire", 0, NULL, 'e' },
        { "force", 0, NULL, 'f' },
        { "maximum", 1, NULL, 'x' },
        { "minimum", 1, NULL, 'n' },
        { "warning", 1, NULL, 'w' },
        { "inactive", 1, NULL, 'i' },
        { "status", 0, NULL, 'S' },
        { "stdin", 0, NULL, 's' },
        { "help", 0, NULL, '?' },
        { "usage", 0, NULL, 'g' },
        { NULL, 0, NULL, 0 }
    };
    
    while(getopt_long(ac, cmdLine, opts, lopts, NULL) != -1) { }

    if(optind < ac) {
        for (int i = optind; i < ac; i++) {
            for (char** ra = rArgs; *ra != NULL; ra++) {
                if (strcmp(cmdLine[i], *ra) == 0) {
                    return -1;
                }
            }
        }
        return 0;
    } else {
        for (char** ra = rArgs; *ra != NULL; ra++) {
            if (strlen(*ra) == 0) {
                return -1;
            }
        }
        return 0;
    }
}
