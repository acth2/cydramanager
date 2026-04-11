#include "configuration.h"
#include "../utilities/utils.h"
#include "src/exit/exit.h"
#include <stdio.h>
#include <string.h>

char raw_configuration[] = "/etc/cydramanager.d/configuration.conf";

char *getConfigurationPath() { return raw_configuration; }

char *getMirror() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    char* keyword = "mirror=";
    bool validate = false;
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the mirror key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
}

char *getUpdateArchive() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char* keyword = "update-archive-db=";
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the update-archive key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
}

char *getUpdateArchiveInstructions() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char* keyword = "update-archive-db-instruc=";
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the update-archive-instructions key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
}

char *getTmpFolder() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char* keyword = "tmp-folder=";
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the tmp-folder key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
}

char *getParallelJobs() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char* keyword = "parallel-jobs=";
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the parallel-jobs key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
}

char *getDepedencyHandling() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char* keyword = "dependency-handling=";
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the dependency-handling key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
}

char *getDefaultArg() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        fclose(configuration);
        set_exit(1);
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char* keyword = "default-arg=";
    static char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));
            validate = true;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the default-arg key is not found in the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        return NULL;
    }

    return output;
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
