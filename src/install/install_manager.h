#ifndef INSTALL_MANAGER_H
#define INSTALL_MANAGER_H

#include "src/utilities/software.h"
#include <stdbool.h>

#define MAXIMUM_LINES  512
#define MAXIMUM_LENGTH 256

typedef struct {
    SoftwareDB software_database;
    bool return_error;
} InstallDB;

bool install_software(char *package_name, bool dependency);

#endif
