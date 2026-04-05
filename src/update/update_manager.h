#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include <stdbool.h>
#include "../utilities/software.h"

#define MAXIMUM_LINES  512
#define MAXIMUM_LENGTH 256

typedef struct {
    int*       outdated_index;
    int        outdated_size;
    SoftwareDB updated_db;
} UpdatedDB;

SoftwareDB get_current_database(char* db);
UpdatedDB get_updated_database(SoftwareDB old_instance);
void update_package(UpdatedDB package, int index, bool download_dbs);
bool apply_software_db(SoftwareDB db);

#endif
