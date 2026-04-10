#ifndef ARG_H
#define ARG_H

#include <string.h>
#include <stdbool.h>

static const char *help_args[] =      { "--help",      "-help",      "help",      "-h" };
static const char *update_args[] =    { "--update",    "-update",    "update",    "-u" };
static const char *version_args[] =   { "--version",   "-version",   "version",   "-v" };
static const char *install_args[] =   { "--install",   "-install",   "install",   "-i" };
static const char *uninstall_args[] = { "--remove",    "-remvoe",    "remove",    "-r" };
static const char *debug_args[] =     { "--debug",     "-debug",                  "-d" };
static const char *conf_args[] =      { "--conf"       "-conf",                   "-c" };

enum ARG {
    UNK,
    DEFAULT,

    HELP,
    UPDATE,
    VERSION,
    INSTALL,
    UNINSTALL,

    ARG_DEBUG,
    ARG_CONF
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
    if (contains(arg, conf_args))
        return ARG_CONF;
    return UNK;
}

#endif
