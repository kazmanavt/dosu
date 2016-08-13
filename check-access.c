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
#include <string.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#include <kz_erch.h>

struct Path_s {
    char* path;
    struct Path_s* next;
};
typedef struct Path_s Path_t;

static Path_t* rFiles = NULL;
static Path_t* rDirs = NULL;

/*
 * normalize given path and add it to list of restricted dirs or files
 * depending on its type
 */
char* processPath(char* path) {
    Path_t** root = path[strlen(path) - 1] == '/' ? &rDirs : &rFiles;

    char* rp = NULL;
    EC_NULL( rp = realpath(path, NULL) );

    Path_t* item = NULL;
    EC_NULL( item = malloc(sizeof(Path_t)) );
    item->path = rp;
    item->next = *root;
    *root = item;
    
    return rp;

    EC_CLEAN_SECTION (
        return NULL;
    );
}


/*
 * expand given glob and pass each result to processPath()
 */
int processCfgItem(char* path) {
    glob_t globBuf;
    int state = 0, rc = 0;
    EC_NZERO( rc = glob(path, GLOB_MARK, NULL, &globBuf) );
    state = 1;

    for (int i = 0; i < globBuf.gl_pathc; i++) {
        EC_NULL( processPath(globBuf.gl_pathv[i]) );
    }

    globfree(&globBuf);
    return 0;

    EC_CLEAN_SECTION (
        if (state > 0) {
            globfree(&globBuf);
        }
        if (rc == GLOB_NOMATCH) {
            ec_clean();
            return 0;
        }
        return 1;
    );
}

/*
 * Parse restriction list, extract line by line globs stored in config file
 * then pass it to processCfgItem()
 */
int initCheckAccess(char* confName) {
    FILE* CFG = NULL;
    EC_NULL( CFG = fopen(confName, "rt") );

    char* line = NULL;
    size_t len;

    while (1) {
        EC_NEG1 (getline(&line, &len, CFG) );

        int wsp = strspn(line, " \t");
        line = line + wsp;
        len -= wsp;
        if (line[0] == '#') {
            continue;
        }

        char* path;
        int rc = 0;
        EC_ERRNO(rc = sscanf(line, "%2000ms", &path) );
        if (rc < 1) {
            continue;
        }

        EC_NZERO( processCfgItem(path) );
    }

    return 0;

    EC_CLEAN_SECTION (
        if (CFG != NULL) {
            if (feof(CFG)) {
                ec_clean();
                fclose(CFG);
                return 0;
            } else {
                fclose(CFG);
            }
       }
       return 1;
    );
}

static char** curentCmdLine; /* is set by checkAccess before tests starts */

/* make log record if there was attempt to perform not granted actions */
void logIncident() {
    //char *ts = "YYYY-MM-DD hh:mm:ss";
    char ts[20];
    time_t _ts = time(NULL);
    //strftime(ts, strlen(ts), "%F %T", localtime(&_ts));
    strftime(ts, 20, "%F %T", localtime(&_ts));

    FILE* LOG = fopen("/var/log/dosu", "at");
    if (LOG == NULL) return;
    fprintf(LOG, "[%s]: Attempt to execute [ ", ts);
    for (char** param_p = curentCmdLine; *param_p != NULL; param_p++){
        fprintf(LOG, "%s ", *param_p);
    }
    fprintf(LOG, "]\n");
    fclose(LOG);
}



/*
 * if given path points to restricted file or is located under
 * restricted catalog, then print log about incident and return 1
 * return 0 if everything is OK
 */
int checkPath(char* path) {
    for (Path_t* dir = rDirs; dir != NULL; dir = dir->next) {
        if(strncmp(path, dir->path, strlen(dir->path)) == 0) {
            logIncident();
            return 1;
        }
    }
    for (Path_t* file = rFiles; file != NULL; file = file->next) {
        if(strncmp(path, file->path, strlen(file->path)) == 0) {
            logIncident();
            return 1;
        }
    }
   
    return 0;
}


/* 
 * checke if command line contain files or dirs, which is restricted to access
 */
int checkAccess(char** cmdLine) {
    curentCmdLine = cmdLine;
    for (char** param_p = cmdLine + 1; *param_p != NULL; param_p++) {
        char* path = realpath(*param_p, NULL);
        if ( path == NULL && errno != ENOENT) {
            EC_FAIL;
        }
        if ( path != NULL) {
            EC_NZERO( checkPath(path) );
        }
    }

    return 0;

    EC_CLEAN_SECTION(
        return 1;
    );
}



/*
 * protect from changing root credentials
 */
int checkRootPasswd(char** cmdLine) {
    curentCmdLine = cmdLine;
    int ac = 0;
    for (char** p = cmdLine; *p != NULL; p++) {
        ac++;
    }

    char* opts = ":kdluefx:n:w:i:S?";
    struct option* lopts = {
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

    if(optind < ac && strcmp(cmdLine[optind], "root") != 0) {
        return 0;
    } else {
        logIncident();
        return 1;
    }
}
