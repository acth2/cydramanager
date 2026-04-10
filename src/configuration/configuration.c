#include "configuration.h"
#include "../utilities/utils.h"
#include "src/exit/exit.h"
#include <stdio.h>
#include <string.h>

char raw_configuration[] = "/etc/cydramanager.d/configuration.conf";

char* getConfigurationPath() {
    return raw_configuration;
}

void change_configuration(char *path) {
    FILE *configuration = fopen(path, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               path);

        set_exit(1);
        return;
    }
    fclose(configuration);

    strcpy(raw_configuration, path);
}
