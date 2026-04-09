#include "remove_manager.h"
#include "../utilities/utils.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool remove_software(char *package_name) {
    FILE *check_installed_software = fopen("/etc/cydramanager.d/usdb", "r");

    char search[512];
    snprintf(search, sizeof(search), "%s ", package_name);

    char buffer[512];
    bool package_absent = false;
    while (fgets(buffer, sizeof(buffer), check_installed_software)) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strstr(buffer, search) == 0) {
            package_absent = true;
        }
    }
    char package_path[250];
    snprintf(package_path, sizeof(package_path), "/usr/bin/cydramanager/%s",
             package_name);

    DIR *package_directory = opendir(package_path);
    if (package_absent || is_empty(check_installed_software) ||
        !package_directory) {
        fclose(check_installed_software);
        closedir(package_directory);
        printf(RED "Error: You dont have the package %s installed on your "
                   "system.\n" RESET,
               package_name);
        return false;
    }
    closedir(package_directory);
    fclose(check_installed_software);

    printf(GRAY "-> Preparing the removal of %s\n" RESET, package_name);

    char removal_command[512];
    snprintf(removal_command, sizeof(removal_command), "rm -r %s",
             package_path);
    if (system(removal_command) != 0) {
        printf(
            RED
            "Error: could not delete the installation directory of %s\n" RESET,
            package_name);
        return false;
    }
    printf(GRAY "-> Package %s removed.\n" RESET, package_name);
    printf(GRAY "-> Cleaning symbolics links\n" RESET);

    if (system("find /usr/bin/cydramanager-binaries -xtype l -delete") != 0) {
        printf(RED "Error: could not clean the symbolic links\n" RESET);
        return false;
    }

    printf(GRAY "-> Symbolics links removed for the package %s\n" RESET,
           package_name);
    printf(GRAY "-> Removing package from the database\n" RESET);

    FILE *db_read = fopen("/etc/cydramanager.d/usdb", "r");
    FILE *db_write = fopen("/etc/cydramanager.d/usdb", "w");

    char db_buffer[512];
    while (fgets(db_buffer, sizeof(db_buffer), db_read)) {
        if (strstr(db_buffer, search)) {
            continue;
        }

        fputs(db_buffer, db_write);
    }
    fclose(db_read);
    fclose(db_write);

    printf(YELLOW "## The package %s has been removed from the system.\n" RESET,
           package_name);
    return true;
}
