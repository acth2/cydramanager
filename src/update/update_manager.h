#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include <stdbool.h>

#define MAXIMUM_LINES  512
#define MAXIMUM_LENGTH 256

static char build_instructions      [MAXIMUM_LINES][MAXIMUM_LENGTH];
static char install_instructions    [MAXIMUM_LINES][MAXIMUM_LENGTH];
static char dependency_instructions [256]          [MAXIMUM_LENGTH];
static char download_link           [10]           [MAXIMUM_LENGTH];

typedef enum {
    BUILD,
    INSTALL,
    DEPENDENCY,
    DOWNLOAD,
    NONE
} INSTRUCTION_MODE;

typedef struct {
    char software_name    [100];
    char software_version [100];
} SoftwareMap;

typedef struct {
    int*          software_counter;
    SoftwareMap*  software_map;
} SoftwareDB;

typedef struct {
    int*       outdated_index;
    SoftwareDB updated_db;
} UpdatedDB;

SoftwareDB get_current_database(char* db);
UpdatedDB get_updated_database(SoftwareDB old_instance);
void update_package(UpdatedDB package, int index);
bool apply_software_db(SoftwareDB db);

#endif
