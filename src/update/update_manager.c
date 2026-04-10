#include "update_manager.h"
#include "../exit/exit.h"
#include "../arguments/debug/debug.h"
#include "../utilities/utils.h"
#include "src/main.h"
#include <curl/curl.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

SoftwareDB get_current_database(char *dbPath) {
    SoftwareDB db = {0};

    FILE *fptr;
    fptr = fopen(dbPath, "r");

    if (fptr == NULL) {
        printf(RED "Error: The software database cannot be opened by the "
                   "software.\n" RESET);

        set_exit(1);
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

    printf(GRAY "-> The database have been loaded.\n" RESET);

    fclose(fptr);
    free(fileContent);

    return db;
}

UpdatedDB get_updated_database(SoftwareDB old_instance) {
    check_crash();

    UpdatedDB updated_instance = {0};
    updated_instance.updated_db.software_map =
        malloc(*(old_instance.software_counter) * sizeof(SoftwareMap));

    char *cache_dir = "/tmp/cydramanager.tmp";

    system("rm -rf /tmp/cydramanager.tmp");
    if (mkdir(cache_dir, 0777) == -1) {
        printf(RED "Error: could not create a temporary directory in /tmp "
                   "folder.\n" RESET);

        set_exit(1);
        return updated_instance;
    }

    CURL *curl = curl_easy_init();
    FILE *file = fopen("/tmp/cydramanager.tmp/versions.tar.gz", "wb");

    if (!curl || !file) {
        printf(RED "Error: Unexpected behaviour during the update of the "
                   "databases.\n" RESET);

        set_exit(1);
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
        printf(RED "Error: You are not connected to the internet, the update "
                   "cannot happen.\n" RESET);

        set_exit(1);
        return updated_instance;
    }

    if (cperf != CURLE_OK) {
        printf(RED "Error: an unexpected error occured.\n" RESET);

        set_exit(1);
        return updated_instance;
    }
    fclose(file);

    if (system("tar -xzf /tmp/cydramanager.tmp/versions.tar.gz -C "
               "/tmp/cydramanager.tmp") != 0) {
        printf(RED "Error: Could not extract the database.\n" RESET);

        set_exit(1);
        return updated_instance;
    }
    printf(GRAY "-> Downloaded and extracted database.\n" RESET);

    int outdated_packages = 0;
    int *outdated_index =
        malloc(*(old_instance.software_counter) * sizeof(int));
    for (int i = 0; i < *(old_instance.software_counter); i++) {
        char full_path[100];
        strcpy(full_path, "/tmp/cydramanager.tmp/");
        strcat(full_path, old_instance.software_map[i].software_name);

        FILE *fptr = fopen(full_path, "r");
        if (fptr == NULL) {
            printf("Warning: cannot open database file for %s\n" RESET,
                   old_instance.software_map[i].software_name);
            continue;
        }
        char version[100];
        fscanf(fptr, "%s", version);

        if (strcmp(old_instance.software_map[i].software_version, version) !=
            0) {
            outdated_index[outdated_packages] = i;
            outdated_packages++;
        }
        strcpy(updated_instance.updated_db.software_map[i].software_version,
               version);
        strcpy(updated_instance.updated_db.software_map[i].software_name,
               old_instance.software_map[i].software_name);

        fclose(fptr);
    }
    updated_instance.outdated_index = outdated_index;
    updated_instance.outdated_size = outdated_packages;
    printf(YELLOW "## You have %d outdated packages\n" RESET,
           outdated_packages);

    return updated_instance;
}

bool apply_software_db(SoftwareDB db) {
    struct timeval foo;
    gettimeofday(&foo, NULL);

    char old_db_path_fused[128] = "";
    snprintf(old_db_path_fused, sizeof(old_db_path_fused),
             "/etc/cydramanager.d/sdb_%ld_%ld", foo.tv_sec, foo.tv_usec);

    if (rename("/etc/cydramanager.d/sdb", old_db_path_fused) != 0) {
        printf(RED "Error: could not rename the old database.\n" RESET);

        set_exit(1);
        return false;
    }

    FILE *fptr;
    fptr = fopen("/etc/cydramanager.d/sdb", "w");
    if (fptr == NULL) {
        printf(RED "Error: could not create new database.\n" RESET);

        set_exit(1);
        return false;
    }

    for (int i = 0; i < *(db.software_counter); i++) {
        fprintf(fptr, "%s", db.software_map[i].software_name);
        fprintf(fptr, " ");
        fprintf(fptr, "%s", db.software_map[i].software_version);
        fprintf(fptr, "\n");
    }
    fclose(fptr);
    printf(GRAY "-> Updated the database.\n" RESET);

    return true;
}

static char build_instructions[MAXIMUM_LINES][MAXIMUM_LENGTH];
static char install_instructions[MAXIMUM_LINES][MAXIMUM_LENGTH];
static char dependency_instructions[256][MAXIMUM_LENGTH];
static char download_link[10][MAXIMUM_LENGTH];

typedef enum { BUILD, INSTALL, DEPENDENCY, DOWNLOAD, NONE } INSTRUCTION_MODE;

void update_package(UpdatedDB update_database, int index, bool dependency) {
    if (!dependency) {
        if (mkdir("/tmp/cydramanager.tmp/instructions", 0777) == -1) {
            printf(
                RED
                "Error: could not create the instructions database.\n" RESET);

            set_exit(1);
            return;
        }

        CURL *curl = curl_easy_init();
        FILE *file = fopen(
            "/tmp/cydramanager.tmp/instructions/instructions.tar.gz", "wb");

        if (!curl || !file) {
            printf(RED "Error: Unexpected behaviour during the update of the "
                       "instructions databases.\n" RESET);

            set_exit(1);
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
            printf(RED
                   "Error: You are not connected to the internet, the update "
                   "cannot happen.\n" RESET);

            set_exit(1);
            return;
        }

        if (cperf != CURLE_OK) {
            printf(RED "Error: an unexpected error occured.\n" RESET);

            set_exit(1);
            return;
        }
        fclose(file);

        if (system("tar -xzf "
                   "/tmp/cydramanager.tmp/instructions/instructions.tar.gz -C "
                   "/tmp/cydramanager.tmp/instructions") != 0) {
            printf(
                RED
                "Error: Could not extract the instructions database.\n" RESET);

            set_exit(1);
            return;
        }
        printf(
            GRAY
            "-> Downloaded and extracted the instructions database.\n" RESET);
    }

    char instructions_path[250];
    strcpy(instructions_path, "/tmp/cydramanager.tmp/instructions/");
    strcat(instructions_path,
           update_database.updated_db.software_map[index].software_name);

    FILE *fptr = fopen(instructions_path, "r");
    if (fptr == NULL) {
        printf(RED "Error: cannot open the instructions file to build %s" RESET,
               update_database.updated_db.software_map[index].software_name);

        set_exit(1);
        return;
    }

    char package_directory[500];
    snprintf(package_directory, sizeof(package_directory),
             "/tmp/cydramanager.tmp/instructions/%s_space",
             update_database.updated_db.software_map[index].software_name);

    if (mkdir(package_directory, 0777) == -1) {
        printf(RED "Error: Failed to create the package build directory for "
                   "%s\n" RESET,
               update_database.updated_db.software_map[index].software_name);

        set_exit(1);
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
    char archive_directory[512];
    while (true) {
        if (strlen(download_link[i]) <= 0) {
            i = 0;
            break;
        }

        char archive_name[250];
        char *software_name =
            update_database.updated_db.software_map[index].software_name;
        snprintf(
            archive_name, sizeof(archive_name),
            "/tmp/cydramanager.tmp/instructions/%s_space/package_archive_%d",
            software_name, i);

        download_link[i][strcspn(download_link[i], "\n")] = '\0';

        CURL *curl = curl_easy_init();
        FILE *file = fopen(archive_name, "wb");

        if (!curl || !file) {
            printf(RED "Error: Unexpected behaviour during the update of the "
                       "package %s.\n" RESET,
                   software_name);

            set_exit(1);
            break;
        }
        printf(RESET "Starting the update of the package %s\n", software_name);

        curl_easy_setopt(curl, CURLOPT_URL, download_link[i]);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        CURLcode cperf = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (cperf == CURLE_COULDNT_CONNECT) {
            printf(RED
                   "Error: You are not connected to the internet, the update "
                   "cannot happen.\n" RESET);

            set_exit(1);
            break;
        }

        if (cperf != CURLE_OK) {
            printf(RED "Error: an unexpected error occured.\n" RESET);

            set_exit(1);
            break;
        }
        fclose(file);
        printf(RESET "Downloaded the archive for %s\n", software_name);

        char extract_cmd[412];
        snprintf(extract_cmd, sizeof(extract_cmd),
                 "tar xf %s -C /tmp/cydramanager.tmp/instructions/%s_space",
                 archive_name, software_name);
        if (system(extract_cmd) != 0) {
            printf(
                RED
                "Error: Could not extract the sources archive for %s\n" RESET,
                software_name);

            set_exit(1);
            break;
        }

        char archive_space[256];
        snprintf(archive_space, sizeof(archive_space),
                 "/tmp/cydramanager.tmp/instructions/%s_space", software_name);

        DIR *dir = opendir(archive_space);
        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') {
                continue;
            }

            snprintf(archive_directory, sizeof(archive_directory), "%s/%s",
                     archive_space, entry->d_name);
        }
        closedir(dir);

        chdir(archive_directory);

        i++;
        if (i >= 500) {
            printf(RESET "Warning: Loop max use reached (500).\n");
            break;
        }
    }

    i = 0;

    // dependency_instructions
    while (true) {
        if (strlen(dependency_instructions[i]) <= 0) {
            break;
        }

        dependency_instructions[i][strcspn(dependency_instructions[i], "\n")] =
            '\0';
        char *token;
        char *separator = " ";
        for (int j = 0; j >= *(update_database.updated_db.software_counter);
             j++) {
            token = strtok(dependency_instructions[i], separator);
            if (strcmp(token, update_database.updated_db.software_map[j]
                                  .software_name) != 0)
                continue;
            token = strtok(NULL, separator);
            if (strcmp(token, update_database.updated_db.software_map[j]
                                  .software_version) == 0) {
                printf(
                    RESET "Dependency %s already up to date, skipping\n",
                    update_database.updated_db.software_map[j].software_name);
                break;
            }

            printf(RESET "Updating dependency %s\n",
                   update_database.updated_db.software_map[j].software_name);
            update_package(update_database, j, true);
        }

        i++;
        if (i >= 500) {
            printf(RESET "Warning: Loop max use reached (500).\n");
            break;
        }
    }

    printf(RESET "Building and installing the package %s\n",
           update_database.updated_db.software_map[index].software_name);

    // build_instructions
    i = 0;
    while (true) {
        if (strlen(build_instructions[i]) <= 0) {
            break;
        }

        replace_proc(build_instructions[i]);
        build_instructions[i][strcspn(build_instructions[i], "\n")] = '\0';
        if (!is_debug())
            strcat(build_instructions[i], " > /dev/null 2>&1");

        if (strstr(build_instructions[i], "cd ")) {
            char *cd_directory;
            char full_directory_path[1024];
            cd_directory = strtok(build_instructions[i], " ");
            cd_directory = strtok(NULL, " ");
            snprintf(full_directory_path, sizeof(full_directory_path), "%s/%s",
                     archive_directory, cd_directory);
            chdir(full_directory_path);
        }

        if (system(build_instructions[i]) != 0 && is_debug()) {
            printf(
                RED "Error at build instructions numero %d for %s\n" RESET, i,
                update_database.updated_db.software_map[index].software_name);

            set_exit(1);
            break;
        }

        if (is_debug())
            printf("Success at executing %s at build step.\n",
                   build_instructions[i]);

        i++;
        if (i >= 500) {
            printf(RESET "Warning: Loop max use reached (500).\n");
            break;
        }
    }

    // install_instructions
    i = 0;
    while (true) {
        if (strlen(install_instructions[i]) <= 0) {
            break;
        }

        replace_proc(install_instructions[i]);
        install_instructions[i][strcspn(install_instructions[i], "\n")] = '\0';
        if (!is_debug())
            strcat(install_instructions[i], " > /dev/null 2>&1");

        if (system(install_instructions[i]) != 0 && is_debug()) {
            printf(
                RED "Error at install instructions numero %d for %s\n" RESET, i,
                update_database.updated_db.software_map[index].software_name);

            set_exit(1);
            break;
        }
        if (is_debug())
            printf(RESET "Success at executing %s at install step.\n",
                   install_instructions[i]);

        i++;
        if (i >= 500) {
            printf(RESET "Warning: Loop max use reached (500).\n");
            break;
        }
    }
    char version_path[256];
    snprintf(version_path, sizeof(version_path), "%s%s",
             "/tmp/cydramanager.tmp/",
             update_database.updated_db.software_map[index].software_name);

    FILE *user_db_reader = fopen("/etc/cydramanager.d/sdb", "r");
    FILE *user_db_writer = fopen("/etc/cydramanager.d/sdb_temp", "w");

    FILE *software_version_file = fopen(version_path, "r");
    char updated_version[256];

    char current_line[512];

    char line2remove[512];
    snprintf(line2remove, sizeof(line2remove), "%s ",
             update_database.updated_db.software_map[index].software_name);

    char buffer_updated_version[256];
    while (fgets(buffer_updated_version, sizeof(buffer_updated_version),
                 software_version_file)) {
        if (strlen(buffer_updated_version) <= 0) {
            continue;
        }
        buffer_updated_version[strcspn(buffer_updated_version, "\n")] = '\0';

        strcpy(updated_version, buffer_updated_version);
    }

    while (fgets(current_line, sizeof(current_line), user_db_reader)) {
        if (strstr(current_line, line2remove) != 0) {
            continue;
        }

        fputs(current_line, user_db_writer);
    }

    char line2replace[1024];
    snprintf(line2replace, sizeof(line2replace), "%s %s\n",
             update_database.updated_db.software_map[index].software_name,
             updated_version);

    fputs(line2replace, user_db_writer);

    fclose(user_db_reader);
    fclose(user_db_writer);

    if (rename("/etc/cydramanager.d/sdb_temp", "/etc/cydramanager.d/sdb") !=
        0) {
        printf(RED "Error: could not update the software database for the "
                   "package %s\n" RESET,
               update_database.updated_db.software_map[index].software_name);

        set_exit(1);
        return;
    }

    printf(RESET "Software %s updated.\n",
           update_database.updated_db.software_map[index].software_name);
}
