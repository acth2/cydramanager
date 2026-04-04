#ifndef ARG_H
#define ARG_H

#include <string.h>
#include <stdbool.h>

static const char *help_args[] =      { "--help",      "-help",      "help",      "-h" };
static const char *update_args[] =    { "--update",    "-update",    "update",    "-u" };
static const char *version_args[] =   { "--version",   "-version",   "version",   "-v" };
static const char *install_args[] =   { "--install",   "-install",   "install",   "-i" };
static const char *uninstall_args[] = { "--uninstall", "-uninstall", "uninstall", "-u" };
static const char *debug_args[] =     { "--debug",     "-debug",                  "-d" };

enum ARG {
    UNK,
    DEFAULT,

    HELP,
    UPDATE,
    VERSION,
    INSTALL,
    UNINSTALL,

    ARG_DEBUG
};

static bool CONTAINS_IMPL(const char *arg, const char *options[], size_t n) {
    for (size_t i = 0; i < n; i++)
        if (strcmp(arg, options[i]) == 0) return true;
    return false;
}

#define contains(arg, options) CONTAINS_IMPL(arg, options, sizeof(options)/sizeof(options[0]))

static inline enum ARG arg2enum(const char *arg) {
    if (contains(arg, help_args))
        return HELP;
    if (contains(arg, update_args))
        return UPDATE;
    if (contains(arg, version_args))
        return VERSION;
    if (contains(arg, debug_args))
        return ARG_DEBUG;
    if (contains(arg, install_args))
        return INSTALL;
    if (contains(arg, uninstall_args))
        return UNINSTALL;
    return UNK;
}

#endif
