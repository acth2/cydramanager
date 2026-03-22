#ifndef ARG_H
#define ARG_H

#include <string.h>
#include <stdbool.h>

static const char *help_args[] =    { "--help",    "-help",    "help",    "-h" };
static const char *update_args[] =  { "--update",  "-update",  "update",  "-u" };
static const char *version_args[] = { "--version", "-version", "version", "-v" };

enum ARG {
    ARG_UNK,
    ARG_DEFAULT,

    ARG_HELP,
    ARG_UPDATE,
    ARG_VERSION
};

static bool CONTAINS_IMPL(const char *arg, const char *options[], size_t n) {
    for (size_t i = 0; i < n; i++)
        if (strcmp(arg, options[i]) == 0) return true;
    return false;
}

#define contains(arg, options) CONTAINS_IMPL(arg, options, sizeof(options)/sizeof(options[0]))

static inline enum ARG arg2enum(const char *arg) {
    if (contains(arg, help_args))
        return ARG_HELP;
    if (contains(arg, update_args))
        return ARG_UPDATE;
    if (contains(arg, version_args))
        return ARG_VERSION;
    return ARG_UNK;
}

#endif
