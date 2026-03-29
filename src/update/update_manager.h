#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include <stdbool.h>

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
