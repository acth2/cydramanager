#ifndef SOFTWARE_H
#define SOFTWARE_H

typedef struct {
    char software_name    [100];
    char software_version [100];
} SoftwareMap;

typedef struct {
    int*          software_counter;
    SoftwareMap*  software_map;
} SoftwareDB;

#endif
