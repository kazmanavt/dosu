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
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <kz_erch.h>
#include <jconf.h>
#include <grant-access.h>


#define LIST_COMMAND "list"


void usage() {
    printf("Usage: dosu <command> [options]\n");
    printf("    Executes command from preconfigured set of commands.\n");
    printf("    Selects executable by *command* parameter\n");
    printf("\n    Run 'dosu %s' to check avalable commands and corresponding executables\n\n", LIST_COMMAND);
}


int list(void) {
    printf("Available commands:\n");
    JCFobj cmd;
    EC_ERRNO( cmd = jcf_o1st(NULL, ".commands") );
    while (cmd != NULL) {
        printf("  %s\n", jcf_oname(cmd));
        EC_ERRNO( cmd = jcf_onext(cmd) );
    }

    return 0;
    EC_CLEAN_SECTION(
        return -1;
    );
}


char** prepareArgs(int ac, char** av, char* cmdPath) {
    char **args = NULL;
    EC_NULL( args = malloc(ac * sizeof(char*)) );
    args[0] = cmdPath;
    for (int i = 1; i < ac -1; i++) {
        args[i] = av[i + 1];
    }
    args[ac - 1] = NULL;
    return args;

    EC_CLEAN_SECTION (
        return NULL;
    );
}

int main (int ac, char** av) {
    EC_NEG1( setuid(0) );

    jcf_load("/etc/dosu.conf");

    if (ac < 2) {
        usage();
        return 0;
    }
    if (strncmp(av[1], LIST_COMMAND, strlen(LIST_COMMAND)) == 0) {
        list();
        return 0;
    }

    JCFobj cmd;
    EC_ERRNO( cmd = jcf_o1st(NULL, ".commands") );
    while (cmd != NULL) {
        if (strcmp(av[1], jcf_oname(cmd)) == 0) {
            char** cmdLine = NULL;
            char* path;
            EC_ERRNO( path = jcf_s(cmd, ".path") );
            EC_NULL( cmdLine = prepareArgs(ac, av, path) );
            EC_NEG1( grantAccess(cmdLine, cmd) );
            jcf_free();

            EC_NEG1( execv(cmdLine[0], cmdLine) );
        }
        EC_ERRNO( cmd = jcf_onext(cmd) );
    }

    usage();

    return 0;

    EC_CLEAN_SECTION (
      ec_print("");
    );
    return 1;
}


