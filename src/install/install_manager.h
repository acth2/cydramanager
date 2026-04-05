#ifndef INSTALL_MANAGER_H
#define INSTALL_MANAGER_H

#include "src/utilities/software.h"
#include <stdbool.h>

typedef struct {
    SoftwareDB software_database;
    bool return_error;
} InstallDB;

bool install_software(char *package_name);

#endif
