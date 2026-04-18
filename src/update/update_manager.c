#include "update_manager.h"
#include "../arguments/debug/debug.h"
#include "../configuration/configuration.h"
#include "../exit/exit.h"
#include "../utilities/utils.h"
#include "../main.h"
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

    char *cache_dir = getTmpFolder();
    char cache_dir_clean_cmd[512];
    snprintf(cache_dir_clean_cmd, sizeof(cache_dir_clean_cmd), "rm -r %s",
             cache_dir);

    system(cache_dir_clean_cmd);
    if (mkdir(cache_dir, 0777) == -1) {
        printf(RED "Error: could not create a temporary directory\n" RESET);

        set_exit(1);
        return updated_instance;
    }

    CURL *curl = curl_easy_init();
    char versions_dest[512];

    snprintf(versions_dest, sizeof(versions_dest), "%s/versions.tar.gz",
             cache_dir);

    FILE *file = fopen(versions_dest, "wb");

    if (!curl || !file) {
        printf(RED "Error: Unexpected behaviour during the update of the "
                   "databases.\n" RESET);

        set_exit(1);
        return updated_instance;
    }

    curl_easy_setopt(curl, CURLOPT_URL, getUpdateArchive());

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

    char extract_update_db_command[512];
    snprintf(extract_update_db_command, sizeof(extract_update_db_command),
             "tar -xzf %s/versions.tar.gz -C %s", cache_dir, cache_dir);

    if (system(extract_update_db_command) != 0) {
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
        strcpy(full_path, cache_dir);
        strcat(full_path, "/");
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

    mkdir("/etc/cydramanager.d/archives", 0777);
    long long archives_size_bytes =
        get_dir_size("/etc/cydramanager.d/archives");
    double archives_size_mo = archives_size_bytes / (1024.0 * 1024.0);

    if (archives_size_mo >= 10) {
        printf(RESET "Cleaning the archives directory\n");
        if (system("rm -r /etc/cydramanager.d/archives/sdb_*") != 0) {
            printf(RESET "Warning: could not clean the "
                         "/etc/cydramanager.d/archives directory\n");
        }
    }

    char old_db_path_fused[128] = "";
    snprintf(old_db_path_fused, sizeof(old_db_path_fused),
             "/etc/cydramanager.d/archives/sdb_%ld_%ld", foo.tv_sec,
             foo.tv_usec);

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

typedef enum { BUILD, INSTALL, DEPENDENCY, DOWNLOAD, NONE } INSTRUCTION_MODE;

char (*build_instructions)[MAXIMUM_LENGTH];
char (*install_instructions)[MAXIMUM_LENGTH];
char (*dependency_instructions)[MAXIMUM_LENGTH];
char (*download_link)[MAXIMUM_LENGTH];

void on_update_crash() {
    free(build_instructions);
    free(install_instructions);
    free(dependency_instructions);
    free(download_link);
}

void update_package(UpdatedDB update_database, int index, bool dependency) {
    char instructions_directory[256];
    snprintf(instructions_directory, sizeof(instructions_directory),
             "%s/instructions", getTmpFolder());

    build_instructions = calloc(MAXIMUM_LINES, MAXIMUM_LENGTH);
    install_instructions = calloc(MAXIMUM_LINES, MAXIMUM_LENGTH);
    dependency_instructions = calloc(256, MAXIMUM_LENGTH);
    download_link = calloc(10, MAXIMUM_LENGTH);

    if (!dependency) {
        if (mkdir(instructions_directory, 0777) == -1) {
            printf(
                RED
                "Error: could not create the instructions database.\n" RESET);

            set_exit(1);
            on_update_crash();
            return;
        }

        char instructions_archive[512];
        snprintf(instructions_archive, sizeof(instructions_archive),
                 "%s/instructions.tar.gz", instructions_directory);

        CURL *curl = curl_easy_init();
        FILE *file = fopen(instructions_archive, "wb");

        if (!curl || !file) {
            printf(RED "Error: Unexpected behaviour during the update of the "
                       "instructions databases.\n" RESET);

            set_exit(1);
            on_update_crash();
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, getUpdateArchiveInstructions());

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        CURLcode cperf = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (cperf == CURLE_COULDNT_CONNECT) {
            printf(RED
                   "Error: You are not connected to the internet, the update "
                   "cannot happen.\n" RESET);

            set_exit(1);
            on_update_crash();
            return;
        }

        if (cperf != CURLE_OK) {
            printf(RED "Error: an unexpected error occured.\n" RESET);

            set_exit(1);
            on_update_crash();
            return;
        }
        fclose(file);

        char instructions_extract_command[1024];
        snprintf(instructions_extract_command,
                 sizeof(instructions_extract_command), "tar -xzf %s -C %s",
                 instructions_archive, instructions_directory);

        if (system(instructions_extract_command) != 0) {
            printf(
                RED
                "Error: Could not extract the instructions database.\n" RESET);

            set_exit(1);
            on_update_crash();
            return;
        }
        printf(
            GRAY
            "-> Downloaded and extracted the instructions database.\n" RESET);
    }

    char instructions_path[250];
    strcpy(instructions_path, instructions_directory);
    strcat(instructions_path, "/");
    strcat(instructions_path,
           update_database.updated_db.software_map[index].software_name);

    FILE *fptr = fopen(instructions_path, "r");
    if (fptr == NULL) {
        printf(RED "Error: cannot open the instructions file to build %s" RESET,
               update_database.updated_db.software_map[index].software_name);

        set_exit(1);
        on_update_crash();
        return;
    }

    char package_directory[400];
    snprintf(package_directory, sizeof(package_directory), "%s/%s_space",
             instructions_directory,
             update_database.updated_db.software_map[index].software_name);

    if (mkdir(package_directory, 0777) == -1) {
        printf(RED "Error: Failed to create the package build directory for "
                   "%s\n" RESET,
               update_database.updated_db.software_map[index].software_name);

        set_exit(1);
        on_update_crash();
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

    char jobs[128];
    if (strcmp(getParallelJobs(), "auto") == 0) {
        long cores = sysconf(_SC_NPROCESSORS_ONLN);

        if (cores < 1) {
            printf(RESET
                   "Warning: could not resolve the numbre of cpu core on the "
                   "system.\n");
            strcpy(jobs, "");
        } else {
            snprintf(jobs, sizeof(jobs), "%ld", (cores));
        }
    } else {
        char *endptr;

        long askedJobs = strtol(getParallelJobs(), &endptr, 10);
        if (*endptr == '\0') {
            if (is_debug())
                printf(RESET "Using %ld jobs as asked on the configuration.\n",
                       askedJobs);

            snprintf(jobs, sizeof(jobs), "%s", getParallelJobs());
        } else {
            printf(RESET "Warning: %s is not valid using auto by default\n",
                   getParallelJobs());
            long cores = sysconf(_SC_NPROCESSORS_ONLN);
            if (cores < 1) {
                printf("Error: could not resolve the numbers of cpu cores on "
                       "the system.\n");
                set_exit(1);
                on_update_crash();
                return;
            } else {
                snprintf(jobs, sizeof(jobs), "%ld", (cores / 2));
            }
        }
    }

    // download_link
    int i = 0;
    char archive_directory[750];
    while (true) {
        if (strlen(download_link[i]) <= 0) {
            i = 0;
            break;
        }

        char archive_name[450];
        char *software_name =
            update_database.updated_db.software_map[index].software_name;
        snprintf(archive_name, sizeof(archive_name), "%s/package_archive_%d",
                 package_directory, i);

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

        char extract_cmd[870];
        snprintf(extract_cmd, sizeof(extract_cmd), "tar xf %s -C %s",
                 archive_name, package_directory);
        if (system(extract_cmd) != 0) {
            printf(
                RED
                "Error: Could not extract the sources archive for %s\n" RESET,
                software_name);

            set_exit(1);
            break;
        }

        char archive_space[450];
        snprintf(archive_space, sizeof(archive_space), "%s", package_directory);

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
    if (getDepedencyHandling() != IGNORE) {
        while (true) {
            if (strlen(dependency_instructions[i]) <= 0) {
                break;
            }

            dependency_instructions[i][strcspn(dependency_instructions[i],
                                               "\n")] = '\0';
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
                    printf(RESET "Dependency %s already up to date, skipping\n",
                           update_database.updated_db.software_map[j]
                               .software_name);
                    break;
                }

                printf(
                    RESET "Updating dependency %s\n",
                    update_database.updated_db.software_map[j].software_name);
                if (getDepedencyHandling() == ASK) {
                    char resp;

                    do {
                        printf(YELLOW "-> Do you want to install the "
                                      "dependency %s? (y/n): " RESET,
                               update_database.updated_db.software_map[j]
                                   .software_name);
                        scanf(" %c", &resp);
                    } while (resp != 'y' && resp != 'Y' && resp != 'n' &&
                             resp != 'N');

                    if (resp == 'n' || resp == 'N') {
                        printf(GRAY "Skipping dependency.\n" RESET);
                        i++;
                        break;
                    }
                }
                update_package(update_database, j, true);
            }

            i++;
            if (i >= 500) {
                printf(RESET "Warning: Loop max use reached (500).\n");
                break;
            }
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

        replace_proc(build_instructions[i], jobs);
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

        replace_proc(install_instructions[i], jobs);
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
    snprintf(version_path, sizeof(version_path), "%s/%s", getTmpFolder(),
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
        on_update_crash();
        return;
    }

    printf(RESET "Software %s updated.\n",
           update_database.updated_db.software_map[index].software_name);
}
