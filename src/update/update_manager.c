#include "update_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// make a database file content into a SoftwareDB variable.
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
        printf("Software: %s, Version: %s", db.software_map[i].software_name,
               db.software_map[i].software_version);
        i++;
    }
    *(db.software_counter) = i;
    
    printf("Total software: %d\n", *(db.software_counter));
    db.software_counter = realloc(db.software_counter, sizeof(int) * (i + 1));
    db.software_map     = realloc(db.software_map, sizeof(SoftwareMap) * (i + 1));

    fclose(fptr);
    free(fileContent);

    return db;
}

// create a new database file and archive the predecessor
bool create_software_db(SoftwareDB db) {
    struct timeval foo;
    gettimeofday(&foo, NULL);

    char current_time[20];
    char old_db_path_raw[50] = "/etc/cydramanager.d/sdb";
    char old_db_path_fused[50] = "";

    sprintf(current_time, "%ld", foo.tv_usec);
    strcpy(old_db_path_fused, old_db_path_raw);
    strcat(old_db_path_fused, current_time);

    if (rename("/etc/cydramanager.d/sdb", old_db_path_fused) != 0) {
        printf("Error: could not rename the old database.\n");
        return false;
    }
    printf("Renamed old database, creating new one.\n");

    FILE *fptr;
    fptr = fopen("/etc/cydramanager.d/sdb", "w");
    if (fptr == NULL) {
        printf("Error: could not create new database.\n");
        return false;
    }

    for (int i = 0; i <= ( *(db.software_counter) - 1 ); i++) {
        fprintf(fptr, db.software_map[i].software_name);
        fprintf(fptr, " ");
        fprintf(fptr, db.software_map[i].software_version);
        printf("Wrote for: %s version %s", db.software_map[i].software_name, db.software_map[i].software_version);
    }

    fclose(fptr);

    return false;
}
