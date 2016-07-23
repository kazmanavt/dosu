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

#define MAX_LINE (1000)
#define LIST_COMMAND "list"

// Config line repr
//    cmd - command to be given
//    path - program to be executed
struct CmdSpec_S {
    char* cmd;
    char* path;
};
typedef struct CmdSpec_S CmdSpec;

void usage() {
    printf("Usage: dosu <command> [options]\n");
    printf("    Executes command from preconfigured set of commands.\n");
    printf("    Selects executable by *command* parameter\n");
    printf("\n    Run 'dosu %s' to check avalable commands and corresponding executables\n\n", LIST_COMMAND);
}
void list(CmdSpec** cmds) {
    printf("Available commands:\n");
    for (CmdSpec** cmd = cmds; *cmd != NULL; cmd++) {
        printf("   %s: %s\n", (*cmd)->cmd, (*cmd)->path);
    }
}

// check alternate config existance
// used only during development
#ifdef DEBUG
char* findConfig() {
    struct stat junk;
    char* confFileName = getenv("DOSU_CONF");
    if (stat(confFileName, &junk) != 0) {
        confFileName = "/etc/dosu.conf";
        if (stat(confFileName, &junk) != 0) {
            printf("ERR: No config file\n");
            exit(1);
        }
    }
    return confFileName;
}
#endif

// loads commands granted to execute from config
// builds buffer with pointers to command specification, NULl terminated
CmdSpec** readConfig(char* fName) {
    FILE* CFG = NULL;
    EC_NULL( CFG = fopen(fName, "rt") );

    char* line = NULL;
    EC_NULL( line = malloc((MAX_LINE + 2) * sizeof(char)) );
    size_t len = MAX_LINE;

    size_t num_pre = 20;
    CmdSpec** cmds = NULL;
    EC_NULL( cmds = malloc(num_pre * sizeof(CmdSpec*)) );
    
    size_t num = 0;
    while (1) {
        EC_NEG1 (getline(&line, &len, CFG) );

        int wsp = strspn(line, " \t");
        line = line + wsp;
        len -= wsp;
        if (line[0] == '#') {
            continue;
        }

        char* cmd;
        char* path;
        int rc = 0;
        EC_ERRNO(rc = sscanf(line, "%50ms %200ms", &cmd, &path) );
        if (rc < 2) {
            continue;
        }
        if (num >= num_pre) {
            EC_REALLOC(cmds, num_pre + 20);
            num_pre += 20;   
        }
        CmdSpec* item = NULL;
        EC_NULL( item = malloc(sizeof(CmdSpec)) );
        item->cmd = cmd;
        item->path = path;
        cmds[num] = item;
        ++num;
    }

    EC_CLEAN_SECTION (
        if (CFG != NULL) {
            if (feof(CFG)) {
                ec_clean();
                fclose(CFG);
                cmds[num] = NULL;
                return cmds;
            } else {
                fclose(CFG);
            }
       }
       return NULL;
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
    CmdSpec** cmds = NULL;
    EC_NULL( cmds = readConfig("/etc/dosu.conf") );

    if (ac < 2) {
        usage();
        return 0;
    }
    if (strncmp(av[1], LIST_COMMAND, strlen(LIST_COMMAND)) == 0) {
        list(cmds);
        return 0;
    }

    for (CmdSpec** cmd = cmds; *cmd != NULL; cmd++) {
        if (strncmp((*cmd)->cmd, av[1], 200) == 0) {
            char** cmdline = NULL;
            EC_NULL( cmdline = prepareArgs(ac, av, (*cmd)->path) );
            EC_NEG1( execv((*cmd)->path, prepareArgs(ac, av, (*cmd)->path)) );
        }
    }
    usage(cmds);

    return 0;

    EC_CLEAN_SECTION (
      ec_print("");
    );
    return 1;
}


