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

#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <getopt.h>

#include <kz_erch.h>
#include <opts-check.h>

char* buildShortOptsString(Opts_t** rOpts) {
    char* opts;
    EC_NULL( opts = malloc(1000 * sizeof(char)) );
    opts[0] = '\0';

    for (Opts_t** ropt = rOpts; *ropt != NULL; ropt++) {
        strncat(opts, ropt[0]->name, 1);
        strcat(opts, ":");
    }

    return opts;

    EC_CLEAN_SECTION();

    return NULL;
}

int runOptsCheck(char** cmdLine, Opts_t** rOpts) {
    int ac = 0;
    for (char** part = cmdLine; *part != NULL; part++) ++ac;

    char* sopts;
    EC_NULL( sopts = buildShortOptsString(rOpts) );

    int rc;
    opterr = 0;
    while ((rc = getopt_long(ac, cmdLine, sopts, NULL, NULL)) != -1) {
        for (Opts_t** ropt = rOpts; *ropt != NULL; ropt++) {
            if (rc == ropt[0]->name[0]) {
                regex_t patt;
                EC_RC( regcomp(&patt, ropt[0]->val, REG_EXTENDED | REG_NOSUB) );
                int rc = regexec(&patt, optarg, 0, NULL, 0);
                regfree(&patt);
                if (rc == 0) return -1;
            }
        }
    }

    return 0;

    EC_CLEAN_SECTION();
    return -1;
}
