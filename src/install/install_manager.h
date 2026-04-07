#ifndef INSTALL_MANAGER_H
#define INSTALL_MANAGER_H

#include <stdbool.h>

#define MAXIMUM_LINES  512
#define MAXIMUM_LENGTH 256

bool install_software(char *package_name, bool dependency);

#endif
