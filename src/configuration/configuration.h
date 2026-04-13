#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdbool.h>

char* getConfigurationPath();

char* getSoftwareMirror();
char* getUpdateArchive();
char* getUpdateArchiveInstructions();
char* getTmpFolder();
char* getParallelJobs();
bool  getDepedencyHandling();
char* getDefaultArg();

void change_configuration(char *path);

#endif
