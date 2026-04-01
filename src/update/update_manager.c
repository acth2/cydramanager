#include "update_manager.h"
#include "../utilities/utils.h"
#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

SoftwareDB get_current_database(char *dbPath) {
    SoftwareDB db = {0};

    FILE *fptr;
    fptr = fopen(dbPath, "r");

    if (fptr == NULL) {
        printf(
            "Error: The software database cannot be opened by the software.\n");
        return db;
    }

    db.software_counter = malloc(sizeof(int));
    db.software_map = malloc(sizeof(SoftwareMap) * 1000);

    char *fileContent = malloc(10000);
    // send him to debug prison
    // fgets(fileContent, 10000, fptr);

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
        i++;
    }
    *(db.software_counter) = i;

    db.software_map = realloc(db.software_map, sizeof(SoftwareMap) * (i + 1));

    for (int i = 0; i < *(db.software_counter); i++) {
        int j = 0;
        for (int k = 0; db.software_map[i].software_version[k] != '\0'; k++) {
            if (db.software_map[i].software_version[k] != '\n') {
                db.software_map[i].software_version[j++] =
                    db.software_map[i].software_version[k];
            }
        }
        db.software_map[i].software_version[j] = '\0';
    }

    printf("-> The database have been loaded.\n");

    fclose(fptr);
    free(fileContent);

    return db;
}

UpdatedDB get_updated_database(SoftwareDB old_instance) {
    UpdatedDB updated_instance = {0};
    updated_instance.updated_db.software_map =
        malloc(*(old_instance.software_counter) * sizeof(SoftwareMap));

    char *cache_dir = "/tmp/cydramanager.tmp";

    system("rm -rf /tmp/cydramanager.tmp");
    if (mkdir(cache_dir, 0777) == -1) {
        printf(
            "Error: could not create a temporary directory in /tmp folder.\n");
        return updated_instance;
    }

    CURL *curl = curl_easy_init();
    FILE *file = fopen("/tmp/cydramanager.tmp/versions.tar.gz", "wb");

    if (!curl || !file) {
        printf("Error: Unexpected behaviour during the update of the "
               "databases.\n");
        return updated_instance;
    }

    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://github.com/acth2/cydramanager-db/releases/"
                     "download/13.0/versions.tar.gz");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode cperf = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (cperf == CURLE_COULDNT_CONNECT) {
        printf("Error: You are not connected to the internet, the update "
               "cannot happen.\n");
        return updated_instance;
    }

    if (cperf != CURLE_OK) {
        printf("Error: an unexpected error occured.\n");
        return updated_instance;
    }
    fclose(file);

    if (system("tar -xzf /tmp/cydramanager.tmp/versions.tar.gz -C "
               "/tmp/cydramanager.tmp") != 0) {
        printf("Error: Could not extract the database.\n");
        return updated_instance;
    }
    printf("-> Downloaded and extracted database.\n");

    int outdated_packages = 0;
    for (int i = 0; i < *(old_instance.software_counter); i++) {
        char full_path[100];
        strcpy(full_path, "/tmp/cydramanager.tmp/");
        strcat(full_path, old_instance.software_map[i].software_name);

        FILE *fptr = fopen(full_path, "r");
        if (fptr == NULL) {
            printf("Warning: cannot open database file for %s\n",
                   old_instance.software_map[i].software_name);
            continue;
        }
        char version[100];
        int *outdated_index =
            malloc(*(old_instance.software_counter) * sizeof(int));
        fscanf(fptr, "%s", version);

        if (strcmp(old_instance.software_map[i].software_version, version) !=
            0) {
            outdated_packages++;
            outdated_index[outdated_packages - 1] = i;
        }
        strcpy(updated_instance.updated_db.software_map[i].software_version,
               version);
        strcpy(updated_instance.updated_db.software_map[i].software_name,
               old_instance.software_map[i].software_name);

        updated_instance.outdated_index = outdated_index;
        fclose(fptr);
    }
    printf("## You have %d outdated packages\n", outdated_packages);

    return updated_instance;
}

bool apply_software_db(SoftwareDB db) {
    struct timeval foo;
    gettimeofday(&foo, NULL);

    char old_db_path_fused[128] = "";
    snprintf(old_db_path_fused, sizeof(old_db_path_fused),
             "/etc/cydramanager.d/sdb_%ld_%ld", foo.tv_sec, foo.tv_usec);

    if (rename("/etc/cydramanager.d/sdb", old_db_path_fused) != 0) {
        printf("Error: could not rename the old database.\n");
        return false;
    }

    FILE *fptr;
    fptr = fopen("/etc/cydramanager.d/sdb", "w");
    if (fptr == NULL) {
        printf("Error: could not create new database.\n");
        return false;
    }

    for (int i = 0; i < *(db.software_counter); i++) {
        fprintf(fptr, "%s", db.software_map[i].software_name);
        fprintf(fptr, " ");
        fprintf(fptr, "%s", db.software_map[i].software_version);
        fprintf(fptr, "\n");
    }
    fclose(fptr);
    printf("-> Updated the database.\n");

    return true;
}

static char build_instructions[MAXIMUM_LINES][MAXIMUM_LENGTH];
static char install_instructions[MAXIMUM_LINES][MAXIMUM_LENGTH];
static char dependency_instructions[256][MAXIMUM_LENGTH];
static char download_link[10][MAXIMUM_LENGTH];

void update_package(UpdatedDB update_database, int index) {
    if (mkdir("/tmp/cydramanager.tmp/instructions", 0777) == -1) {
        printf("Error: could not create the instructions database.\n");
        return;
    }

    CURL *curl = curl_easy_init();
    FILE *file =
        fopen("/tmp/cydramanager.tmp/instructions/instructions.tar.gz", "wb");

    if (!curl || !file) {
        printf("Error: Unexpected behaviour during the update of the "
               "instructions databases.\n");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://github.com/acth2/cydramanager-db/releases/"
                     "download/13.0/instructions.tar.gz");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode cperf = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (cperf == CURLE_COULDNT_CONNECT) {
        printf("Error: You are not connected to the internet, the update "
               "cannot happen.\n");
        return;
    }

    if (cperf != CURLE_OK) {
        printf("Error: an unexpected error occured.\n");
        return;
    }
    fclose(file);

    if (system("tar -xzf "
               "/tmp/cydramanager.tmp/instructions/instructions.tar.gz -C "
               "/tmp/cydramanager.tmp/instructions") != 0) {
        printf("Error: Could not extract the instructions database.\n");
        return;
    }
    printf("-> Downloaded and extracted the instructions database.\n");

    char instructions_path[250];
    strcpy(instructions_path, "/tmp/cydramanager.tmp/instructions/");
    strcat(instructions_path,
           update_database.updated_db.software_map[index].software_name);

    FILE *fptr = fopen(instructions_path, "r");
    if (fptr == NULL) {
        printf("Error: cannot open the instructions file to build %s",
               update_database.updated_db.software_map[index].software_name);
        return;
    }

    char package_directory[500];
    snprintf(package_directory, sizeof(package_directory),
             "/tmp/cydramanager.tmp/instructions/%s_space",
             update_database.updated_db.software_map[index].software_name);

    if (mkdir(package_directory, 0777) == -1) {
        printf("Error: Failed to create the package build directory for %s\n",
               update_database.updated_db.software_map[index].software_name);
        return;
    }

    char line[512];
    char line_cleaned[512];
    int step = 0;
    INSTRUCTION_MODE mode = NONE;
    while (fgets(line, sizeof(line), fptr) != NULL) {
        strcpy(line_cleaned, line);

        if (strcmp(space_clean(line_cleaned), "build['") == 0) {
            mode = BUILD;
            continue;
        }

        if (strcmp(space_clean(line_cleaned), "install['") == 0) {
            mode = INSTALL;
            continue;
        }

        if (strcmp(space_clean(line_cleaned), "dependency['") == 0) {
            mode = DEPENDENCY;
            continue;
        }

        if (strcmp(space_clean(line_cleaned), "download['") == 0) {
            mode = DOWNLOAD;
            continue;
        }

        if (strcmp(space_clean(line_cleaned), "']") == 0 && mode != NONE) {
            mode = NONE;
            continue;
        }

        switch (mode) {
        case BUILD: {
            strcpy(build_instructions[step], line);
            step++;
            break;
        }

        case INSTALL: {
            strcpy(install_instructions[step], line);
            step++;
            break;
        }

        case DEPENDENCY: {
            strcpy(dependency_instructions[step], line);
            step++;
            break;
        }

        case DOWNLOAD: {
            strcpy(download_link[step], line);
            step++;
            break;
        }

        case NONE:
        default:
            step = 0;
            continue;
        }
    }

    // download_link
    int i = 0;
    while (true) {
        if (strlen(download_link[i]) <= 0) {
            i = 0;
            break;
        }

        char archive_name[250];
        char *software_name = update_database.updated_db.software_map[index].software_name;
        snprintf(
            archive_name, sizeof(archive_name),
            "/tmp/cydramanager.tmp/instructions/%s_space/package_archive_%d",
            software_name, i);

        download_link[i][strcspn(download_link[i], "\n")] = '\0';

        CURL *curl = curl_easy_init();
        FILE *file = fopen(archive_name, "wb");

        if (!curl || !file) {
            printf("Error: Unexpected behaviour during the update of the "
                   "package %s.\n", software_name);
            break;
        }
        printf("Starting the update of the package %s\n", software_name);

        curl_easy_setopt(curl, CURLOPT_URL, download_link[i]);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        CURLcode cperf = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (cperf == CURLE_COULDNT_CONNECT) {
            printf("Error: You are not connected to the internet, the update "
                   "cannot happen.\n");
            break;
        }

        if (cperf != CURLE_OK) {
            printf("Error: an unexpected error occured.\n");
            break;
        }
        fclose(file);
        printf("Downloaded the archive for %s\n", software_name);

        char extract_cmd[412];
        snprintf(extract_cmd, sizeof(extract_cmd), "tar xf %s -C /tmp/cydramanager.tmp/instructions/%s_space", archive_name, software_name);
        if (system(extract_cmd) != 0) {
            printf("Error: Could not extract the sources archive for %s\n", software_name);
            break;
        }

        char archive_directory[512];
        char archive_space[256];
        snprintf(archive_space, sizeof(archive_space), "/tmp/cydramanager.tmp/instructions/%s_space", software_name);

        DIR *dir = opendir(archive_space);
        struct dirent *entry;

        while ( (entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') {
                continue;
            }

            snprintf(archive_directory, sizeof(archive_directory), "%s/%s", archive_space, entry->d_name);
        }
        closedir(dir);

        chdir(archive_directory);

        i++;
        if (i >= 500) {
            printf("Warning: Loop max use reached (500).\n");
            break;
        }
    }

    // build_instructions
    i = 0;
    while (true) {
        if (strlen(build_instructions[i]) <= 0) {
            break;
        }

        replace_proc(build_instructions[i]);

        if (system(build_instructions[i]) != 0) {
            printf("Error at build instruction numero %d\n", i);
            break;
        }
        printf("Success at executing %s at build step.\n", build_instructions[i]);

        i++;
        if (i >= 500) {
            printf("Warning: Loop max use reached (500).\n");
            break;
        }
    }

    i = 0;
    while (true) {
        if (strlen(install_instructions[i]) <= 0) {
            break;
        }

        replace_proc(install_instructions[i]);

        if (system(install_instructions[i]) != 0) {
            printf("Error at build instruction numero %d\n", i);
        }
        printf("Success at executing %s at install step.\n", install_instructions[i]);

        i++;
        if (i >= 500) {
            printf("Warning: Loop max use reached (500).\n");
            break;
        }
    }
} 
