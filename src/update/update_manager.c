#include "update_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SoftwareDB get_software_database() {
    SoftwareDB db = {0};

    FILE *fptr;
    fptr = fopen("/etc/cydramanager.d/sdb", "r");

    if (fptr == NULL) {
        printf(
            "Error: The software database cannot be opened by the software.\n");
        return db;
    }
    
    db.software_counter = malloc(sizeof(int) * 1000);
    db.software_map = malloc(sizeof(SoftwareMap) * 1000);    

    char *fileContent = malloc(10000);
    fgets(fileContent, 10000, fptr);

    int i = 0;
    while (fgets(fileContent, 10000, fptr)) {
        char separator[2] = " ";
        char *token;

        token = strtok(fileContent, separator);
        strcpy(db.software_map[i].software_name, token);

        while (token != NULL) {
            strcpy(db.software_map[i].software_version, token);
            token = strtok(NULL, separator);
        } 
        printf("Software: %s, Version: %s", db.software_map[i].software_name, db.software_map[i].software_version);
        i++;
    }
    db.software_map = realloc(db.software_map, sizeof(SoftwareMap) * (i + 1));

    fclose(fptr);
    free(fileContent);

    return db;
}
/*
bool apply_software_db(SoftwareDB db) {

    return false;
}
*/
