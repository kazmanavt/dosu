#ifndef OPTS_CHECK_H_
#define OPTS_CHECK_H_

struct Opts_s {
    char* name;
    int isLong;
    char* val;
};

typedef struct Opts_s Opts_t;

int runOptsCheck(char** cmdLine, Opts_t** rOpts);

#endif // OPTS_CHECK_H_

