#ifndef CONFIGURATION_H
#define CONFIGURATION_H

char* getConfigurationPath();

char* getSoftwareMirror();
char* getUpdateArchive();
char* getUpdateArchiveInstructions();
char* getTmpFolder();
char* getParallelJobs();
char* getDepedencyHandling();
char* getDefaultArg();

void change_configuration(char *path);

#endif
