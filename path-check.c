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
#include <string.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <kz_erch.h>

struct Path_s {
    ino_t ino;
    dev_t dev;
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

    struct stat stt;
    EC_NEG1( stat(path, &stt) );


    Path_t* item = NULL;
    EC_NULL( item = malloc(sizeof(Path_t)) );
    item->dev = stt.st_dev;
    item->ino = stt.st_ino;
    item->next = *root;
    *root = item;
    
    return 0;

    EC_CLEAN_SECTION (
        return -1;
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
        EC_NEG1( processPath(globBuf.gl_pathv[i]) );
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
        return -1;
    );
}

int buildRestrictTables(char** globs) {
    for (char** g = globs; *g != NULL; g++) {
        EC_NEG1( processCfgItem(*g) );
    }

    return 0;

    EC_CLEAN_SECTION();
    return -1;
}

void freeTable(Path_t* table) {
    for (Path_t* rm = table; rm != NULL;) {
        Path_t* tmp = rm->next;
        free(rm);
        rm = tmp;
    }
}

void freeTables(void) {
    freeTable(rFiles);
    freeTable(rDirs);
}


bool exactMatch(char* ptc, Path_t* table) {
    struct stat stt;
    if (stat(ptc, &stt) == -1) return false;
    for (Path_t* r = table; r != NULL; r = r->next) {
        if (stt.st_dev == r->dev && stt.st_ino == r->ino) return true;
    }
    return false;
}

char* dn(char* p) {
    size_t len = strlen(p);
    while (p[len - 1] == '/') {
        --len;
        p[len] = '\0';
    }
    char* end = strrchr(p, '/');
    if (end != NULL) {
        *end = '\0';
        return p;
    } else {
        return NULL;
    }
}

char* getAbsPath(char* _ptc) {
    char* cwd = getcwd(NULL, 0);
    size_t len  = strlen(_ptc);
    if (len < 1) return cwd;
    char *ptc;
    if (_ptc[0] != '/') {
        if (cwd == NULL) return NULL;
        ptc = malloc((len + strlen(cwd) + 2) * sizeof(char));
        strcpy(ptc, cwd);
        strcat(ptc, "/");
        strcat(ptc, _ptc);
    } else {
        ptc = strdup(_ptc);
    }
    return ptc;
}

char* getRealPart(char* ptc) {
    struct stat stt;
    while(ptc != NULL && stat(ptc, &stt) == -1)  ptc = dn(ptc);
    char* p = realpath(ptc, NULL);
    free(ptc);
    ec_clean();
    return p;
}

bool underRestrictedFolder(char* _ptc) {
    char* ptc = getAbsPath(_ptc);
    if (ptc == NULL) return true;

    ptc = getRealPart(ptc);
    if (ptc == NULL) return true;
    
    while(ptc != NULL) {
        if (exactMatch(ptc, rDirs)) return true;
        ptc = dn(ptc);
    }

    free(ptc);
    freeTables();
    return false;
}

int runPathCheck(char** cmdLine, char** rPaths) {
//    printf("Check: '");
//    for (char** p = cmdLine; *p != NULL; p++) {
//        printf(" %s", *p);
//    }
//    printf("'\nAgainst:");
//    for (char** p = rPaths; *p != NULL; p++) {
//        printf("\n  %s", *p);
//    }
//    printf("\n");
    EC_NEG1( buildRestrictTables(rPaths) );

    for (char** arg = cmdLine+1; *arg != NULL; arg++) {
        if (exactMatch(*arg, rFiles)) return -1;
        if (underRestrictedFolder(*arg)) return -1;
    }

    return 0;

    EC_CLEAN_SECTION();
    return -1;
}

