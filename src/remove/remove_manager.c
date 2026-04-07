#include "remove_manager.h"
#include <stdio.h>
#include <string.h>
#include "../utilities/utils.h"

bool remove_software(char *package_name) {
    FILE *check_installed_software = fopen("/etc/cydramanager.d/usdb", "r");

    char buffer[512];
    bool package_absent = false;
    while (fgets(buffer, sizeof(buffer), check_installed_software)) {
        if (strstr(buffer, package_name) == 0) {
            package_absent = true;
        }
    }

    if (package_absent || is_empty(check_installed_software)) {
        fclose(check_installed_software);
        printf(
            "Error: You dont have the package %s installed on your system.\n",
            package_name);
        return false;
    }
    fclose(check_installed_software);

    return true;
}
