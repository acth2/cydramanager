#include "remove_manager.h"
#include "../utilities/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

bool remove_software(char *package_name) {
    FILE *check_installed_software = fopen("/etc/cydramanager.d/usdb", "r");

    char search[512];
    snprintf(search, sizeof(search), "%s ", package_name);

    char buffer[512];
    bool package_absent = false;
    while (fgets(buffer, sizeof(buffer), check_installed_software)) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strstr(buffer, search) == 0) {
            printf("%s not equal as %s\n", buffer, search);
            package_absent = true;
        }
    }
    char package_path[250];
    snprintf(package_path, sizeof(package_path), "/usr/bin/cydramanager/%s", package_name);

    DIR *package_directory = opendir(package_path);
    if (package_absent || is_empty(check_installed_software) || !package_directory) {
        fclose(check_installed_software);
        closedir(package_directory);
        printf(
            "Error: You dont have the package %s installed on your system.\n",
            package_name);
        return false;
    }
    closedir(package_directory);
    fclose(check_installed_software);

    printf("-> Preparing the removal of %s\n", package_name);

    char removal_command[512];
    snprintf(removal_command, sizeof(removal_command),
             "rm -r %s", package_path);
    if (system(removal_command) != 0) {
        printf("Error: could not delete the installation directory of %s\n",
               package_name);
        return false;
    }
    printf("-> Package %s removed.\n", package_name);
    printf("-> Cleaning symbolics links\n");

    if (system("find /usr/bin/cydramanager-binaries -xtype l -delete") != 0) {
        printf("Error: could not clean the symbolic links\n");
        return false;
    }

    printf("-> Symbolics links removed for the package %s\n", package_name);
    printf("-> Removing package from the database\n");

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

    printf("## The package %s has been removed from the system.\n",
           package_name);
    return true;
}
