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

SoftwareDB get_software_database();


#endif
