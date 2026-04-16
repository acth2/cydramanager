#include "configuration.h"
#include "../exit/exit.h"
#include "../main.h"
#include "../utilities/utils.h"
#include <stdio.h>
#include <string.h>

char raw_configuration[] = "/etc/cydramanager.d/configuration.conf";

char *getConfigurationPath() { return raw_configuration; }

char *getSoftwareMirror() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    char buffer[512];
    char *keyword = "software-mirror=";
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
        check_crash();
        return NULL;
    }

    return output;
}

char *getUpdateArchive() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char *keyword = "update-archive-db=";
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
        printf(
            RED
            "Error: the update-archive key is not found in the configuration "
            "%s\n" RESET,
            raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    return output;
}

char *getUpdateArchiveInstructions() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char *keyword = "update-archive-db-instruc=";
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
        printf(RED "Error: the update-archive-instructions key is not found in "
                   "the configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    return output;
}

char *getTmpFolder() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char *keyword = "tmp-folder=";
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
        printf(RED
               "Error: the tmp-folder key is not found in the configuration "
               "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    return output;
}

char *getParallelJobs() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char *keyword = "parallel-jobs=";
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
        printf(RED
               "Error: the parallel-jobs key is not found in the configuration "
               "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    return output;
}

enum DEPENDENCY_HANDLING getDepedencyHandling() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NORMAL;
    }

    char buffer[512];
    bool validate = false;
    static enum DEPENDENCY_HANDLING result = NORMAL;
    char *keyword = "dependency-handling=";
    char output[512];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strncmp(buffer, keyword, strlen(keyword)) == 0) {
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(output, buffer + strlen(keyword));

            validate = true;
            if (strcmp(output, "install") == 0) {
                break;
            }

            if (strcmp(output, "ask") == 0) {
                result = ASK;
                break;
            }

            result = IGNORE;
            break;
        }
    }
    fclose(configuration);

    if (!validate) {
        printf(RED "Error: the dependency-handling key is not found in the "
                   "configuration "
                   "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NORMAL;
    }

    return result;
}

char *getDefaultArg() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        printf(RED "Error: the configuration file %s isnt correct.\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    char buffer[512];
    bool validate = false;
    char *keyword = "default-arg=";
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
        printf(RED
               "Error: the default-arg key is not found in the configuration "
               "%s\n" RESET,
               raw_configuration);

        set_exit(1);
        check_crash();
        return NULL;
    }

    return output;
}

int getCustomMirrorsCounter() {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        set_exit(1);
        check_crash();
    }

    int detected_mirrors = 0;
    char buffer[2048];
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strstr(buffer, "custom-mirror=") != NULL) {
            detected_mirrors++;
        }
    }
    fclose(configuration);

    return detected_mirrors;
}

char *getCustomMirror(int index) {
    FILE *configuration = fopen(raw_configuration, "r");
    if (!configuration) {
        set_exit(1);
        check_crash();
    }

    char buffer[2048];
    static char mirror[2050];
    int current_instance = 0;
    while (fgets(buffer, sizeof(buffer), configuration)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (strstr(buffer, "custom-mirror") != NULL) {
            if (current_instance == index) {
                strcpy(mirror, buffer);
                char *token = strtok(mirror, "=");
                token = strtok(NULL, "=");
                if (token == NULL) break;
                token[strcspn(token, "\n")] = '\0';
                strcpy(mirror, token);
                break;
            }
            current_instance++;
        }
    }

    return mirror;
}

void change_configuration(char *path) {
    FILE *configuration = fopen(path, "r");
    if (!configuration) {
        set_exit(1);
        check_crash();
        return;
    }
    fclose(configuration);
    strcpy(raw_configuration, path);
}
