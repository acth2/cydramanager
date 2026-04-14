#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <curl/curl.h>
#include <stdbool.h>

char* getConfigurationPath();

char* getSoftwareMirror();
char* getUpdateArchive();
char* getUpdateArchiveInstructions();
char* getTmpFolder();
char* getParallelJobs();
char* getDefaultArg();

void change_configuration(char *path);

enum DEPENDENCY_HANDLING {
    NORMAL,
    ASK,
    IGNORE
};
enum DEPENDENCY_HANDLING  getDepedencyHandling();

#endif
