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

#include <kz_erch.h>
#include <jconf.h>
#include <args-check.h>
#include <path-check.h>
#include <opts-check.h>

char** buildRestrictedArgsList(JCFobj RA, char** list) {
    int len = jcf_al(RA, NULL), len0 = 0;
    if (list != NULL) {
        for (char** p = list; *p != NULL; p++) {
            ++len0;
        }
    }
    EC_REALLOC( list, ((len0 + len + 1) * sizeof(char*)) );
    list[len0 + len] = NULL;

    for (int i = 0; i < len; i++) {
        list[len0 + i] = (char*) jcf_s( jcf_ai(RA, NULL, i), NULL );
    }

    return list;

    EC_CLEAN_SECTION( return NULL );
}

char** buildRestrictedPathList(JCFobj RP, char** list) {
    int len = jcf_al(RP, NULL), len0 = 0;
    if (list != NULL) {
        for (char** p = list; *p != NULL; p++) {
            ++len0;
        }
    }
    EC_REALLOC( list, ((len0 + len + 1) * sizeof(char*)) );
    list[len0 + len] = NULL;

    for (int i = 0; i < len; i++) {
        list[len0 + i] = (char*) jcf_s( jcf_ai(RP, NULL, i), NULL );
    }

    return list;

    EC_CLEAN_SECTION( return NULL );
}

Opts_t** buildRestrictedOptsList(JCFobj RO, Opts_t** list) {
    int len = jcf_al(RO, NULL), len0 = 0;
    if (list != NULL) {
        for (Opts_t** p = list; *p != NULL; p++) {
            ++len0;
        }
    }
    EC_REALLOC( list, ((len0 + len + 1) * sizeof(Opts_t*)) );
    list[len0 + len] = NULL;

    for (int i = 0; i < len; i++) {
        Opts_t* item;
        EC_NULL( item = malloc(sizeof(Opts_t)) );
        int isLong;
        char* val;
        char* name;
        EC_ERRNO( name = (char*) jcf_s( jcf_ai(RO, NULL, i), ".name" ) );
        EC_ERRNO( isLong = jcf_b( jcf_ai(RO, NULL, i), ".long" ) );
        EC_ERRNO( val = (char*) jcf_s( jcf_ai(RO, NULL, i), ".val" ) );

        *item = (Opts_t){.name = name, .isLong = isLong, .val = val};
        list[len0 + i] = item;
    }

    return list;

    EC_CLEAN_SECTION( return NULL );
}

int grantAccess(char** cmdLine, JCFobj cmd) {
    JCFobj R;
    EC_ERRNO( R = jcf_a(cmd, ".arestrict") );
    if (R != NULL) {
        char** raList;
        EC_NULL( raList = buildRestrictedArgsList(R, NULL) );
        EC_ERRNO( R = jcf_a(NULL, ".arestrict") );
        if (R != NULL) {
            EC_NULL( raList = buildRestrictedArgsList(R, raList) );
        }
        EC_NEG1( runArgsCheck(cmdLine, raList) );
    }

    EC_ERRNO( R = jcf_a(cmd, ".frestrict") );
    ec_clean();
    if (R != NULL) {
        char** rpList;
        EC_NULL( rpList = buildRestrictedPathList(R, NULL) );
        EC_ERRNO( R = jcf_a(NULL, ".frestrict") );
        ec_clean();
        if (R != NULL) {
            EC_NULL( rpList = buildRestrictedPathList(R, rpList) );
        }
        EC_NEG1( runPathCheck(cmdLine, rpList) );
    }

    EC_ERRNO( R = jcf_a(cmd, ".orestrict") );
    ec_clean();
    if (R != NULL) {
        Opts_t** roList;
        EC_NULL( roList = buildRestrictedOptsList(R, NULL) );
        EC_ERRNO( R = jcf_a(NULL, ".orestrict") );
        ec_clean();
        if (R != NULL) {
            EC_NULL( roList = buildRestrictedOptsList(R, roList) );
        }
        EC_NEG1( runOptsCheck(cmdLine, roList) );
    }

    return 0;

    EC_CLEAN_SECTION(
    );
    return -1;
}

