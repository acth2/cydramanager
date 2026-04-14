#include "install_manager.h"
#include "../arguments/debug/debug.h"
#include "../utilities/utils.h"
#include "src/configuration/configuration.h"
#include "src/exit/exit.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char build_instructions[MAXIMUM_LINES][MAXIMUM_LENGTH];
static char install_instructions[MAXIMUM_LINES][MAXIMUM_LENGTH];
static char dependency_instructions[256][MAXIMUM_LENGTH];
static char download_link[10][MAXIMUM_LENGTH];

typedef enum { BUILD, INSTALL, DEPENDENCY, DOWNLOAD, NONE } INSTRUCTION_MODE;

bool install_software(char *package_name, bool dependency) {
    if (strlen(package_name) <= 0) {
        printf(RED
               "Error: you need to provide a package name in order to install "
               "it.\n" RESET);

        set_exit(1);
        return false;
    }

    char *cache_dir = getTmpFolder();
    char remove_tmp_command[512];
    snprintf(remove_tmp_command, sizeof(remove_tmp_command), "rm -r %s",
             cache_dir);
    if (!dependency) {
        system(remove_tmp_command);
        if (mkdir(cache_dir, 0777) == -1) {
            printf(RED "Error: could not create a temporary directory in /tmp "
                       "folder.\n" RESET);

            set_exit(1);
            return false;
        }
    }

    char *mirror_link = getSoftwareMirror();
    char package_link[1024] = "";

    snprintf(package_link, sizeof(package_link), "%s/%s", mirror_link,
             package_name);

    char package_path[512];
    char package_instructions_path[512];
    snprintf(package_instructions_path, sizeof(package_instructions_path),
             "%s/instructions", getTmpFolder());
    snprintf(package_path, sizeof(package_path), "%s/%s", getTmpFolder(),
             package_name);

    CURL *curl = curl_easy_init();
    FILE *file = fopen(package_path, "wb");

    if (!curl || !file) {
        printf(RED "Error: Unexpected behaviour during the installation of the "
                   "software %s\n" RESET,
               package_name);

        set_exit(1);
        return false;
    }
    printf(GRAY
           "-> Getting the informations of the package %s from the current "
           "mirror.\n" RESET,
           package_name);

    curl_easy_setopt(curl, CURLOPT_URL, package_link);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode cperf = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (cperf == CURLE_COULDNT_CONNECT) {
        printf(RED
               "Error: You are not connected to the internet, the installation "
               "cannot happen.\n" RESET);

        set_exit(1);
        return false;
    }

    if (cperf != CURLE_OK) {
        printf(RED "Error: an unexpected error occured.\n" RESET);

        set_exit(1);
        return false;
    }
    fclose(file);

    char package_version[256];
    FILE *package_version_file = fopen(package_path, "r");
    fgets(package_version, 256, package_version_file);
    fclose(package_version_file);
    package_version[strcspn(package_version, "\n")] = '\0';

    if (is_debug())
        printf("version: %s, link: %s\n", package_version, package_link);
    if (strcmp(package_version, "404: Not Found") == 0) {
        if (!dependency) {
            printf(
                RED
                "Error: The package you asked to install does not exist in the "
                "current mirror.\n" RESET);

            set_exit(1);
            return false;
        } else {
            printf("Warning: The dependency %s is not found in the current "
                   "mirror\n",
                   package_name);
        }
    }

    printf(YELLOW "## Package %s, version %s found.\n" RESET, package_name,
           package_version);

    FILE *user_software_database = fopen("/etc/cydramanager.d/usdb", "r");

    if (!user_software_database) {
        printf(RED "Error: cannot open the user software database file, does "
                   "it exist?\n" RESET);

        set_exit(1);
        return false;
    }

    char buffer[512];
    char *current_scope;

    while (fgets(buffer, sizeof(buffer), user_software_database)) {
        if (strstr(buffer, package_name) != 0) {
            current_scope = strtok(buffer, " ");
            current_scope = strtok(NULL, " ");

            current_scope[strcspn(current_scope, "\n")] = '\0';
            if (strcmp(current_scope, package_version) == 0) {
                printf(RED
                       "Error: The package %s is already installed and updated "
                       "on your system.\n" RESET,
                       package_name);

                set_exit(1);
                return true;
            }
            printf(RESET
                   "The package %s is already installed but not updated. It "
                   "will be updated.\n",
                   package_name);
        }
    }
    fclose(user_software_database);

    FILE *instructions_file = fopen(package_instructions_path, "wb");
    CURL *instructions_curl = curl_easy_init();
    char instructions_link[512] = "";
    strcpy(instructions_link, mirror_link);
    strcat(instructions_link, "/instructions/");
    strcat(instructions_link, package_name);

    if (!instructions_curl || !instructions_file) {
        printf(RED "Error: Unexpected behaviour during the installation of the "
                   "software %s\n" RESET,
               package_name);

        set_exit(1);
        return false;
    }

    curl_easy_setopt(instructions_curl, CURLOPT_URL, instructions_link);

    curl_easy_setopt(instructions_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(instructions_curl, CURLOPT_WRITEDATA, instructions_file);

    CURLcode cperf_instructions = curl_easy_perform(instructions_curl);
    curl_easy_cleanup(instructions_curl);

    if (cperf_instructions == CURLE_COULDNT_CONNECT) {
        printf(RED
               "Error: You are not connected to the internet, the installation "
               "cannot happen.\n" RESET);

        set_exit(1);
        return false;
    }

    if (cperf_instructions != CURLE_OK) {
        printf(RED "Error: An unexpected error occured.\n" RESET);

        set_exit(1);
        return false;
    }
    fclose(instructions_file);

    char package_directory[512] = "";
    snprintf(package_directory, sizeof(package_directory), "%s/%s_space",
             cache_dir, package_name);

    if (mkdir(package_directory, 0777) == -1) {
        printf(RED "Error: Could not create the package directory in the "
                   "tmp.\n" RESET);

        set_exit(1);
        return false;
    }

    FILE *instructions_reader = fopen(package_instructions_path, "r");
    if (instructions_reader == NULL) {
        printf(RED "Error: cannot open the instructions file to build %s" RESET,
               package_name);

        set_exit(1);
        return false;
    }

    char line[512];
    char line_cleaned[512];
    int step = 0;
    INSTRUCTION_MODE mode = NONE;
    while (fgets(line, sizeof(line), instructions_reader) != NULL) {
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
            snprintf(jobs, sizeof(jobs), "%ld", (cores / 2));
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
                return NULL;
            } else {
                snprintf(jobs, sizeof(jobs), "%ld", (cores / 2));
            }
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
        snprintf(archive_name, sizeof(archive_name),
                 "%s/%s_space/package_archive_%d", cache_dir, package_name, i);

        download_link[i][strcspn(download_link[i], "\n")] = '\0';

        CURL *curl = curl_easy_init();
        FILE *file = fopen(archive_name, "wb");

        if (!curl || !file) {
            printf(RED
                   "Error: Unexpected behaviour during the installation of the "
                   "package %s.\n" RESET,
                   package_name);

            set_exit(1);
            break;
        }
        printf("Starting the update of the package %s\n", package_name);

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
        printf("Downloaded the archive for %s\n", package_name);

        char extract_cmd[1024];
        snprintf(extract_cmd, sizeof(extract_cmd), "tar xf %s -C %s",
                 archive_name, package_directory);
        if (system(extract_cmd) != 0) {
            printf(
                RED
                "Error: Could not extract the sources archive for %s\n" RESET,
                package_name);
            break;
        }

        char archive_space[256];
        snprintf(archive_space, sizeof(archive_space), "%s/%s_space", cache_dir,
                 package_name);

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
            printf("Warning: Loop max use reached (500).\n");
            break;
        }
    }

    i = 0;

    // dependency_instructions
    if (getDepedencyHandling()) {
        while (true) {
            if (strlen(dependency_instructions[i]) <= 0) {
                break;
            }

            printf(GRAY
                   "-> Installing dependency %s for the package %s\n" RESET,
                   dependency_instructions[i], package_name);

            if (getDepedencyHandling() == ASK) {
                char resp;

                do {
                    printf(YELLOW "-> Do you want to install the dependency "
                                  "%s? (y/n)\n" RESET,
                           package_name);
                    scanf(" %c", &resp);
                } while (resp != 'y' && resp != 'Y' && resp != 'n' &&
                         resp != 'N');

                if (resp == 'n' || resp == 'N') {
                    printf(GRAY "Skipping dependency.\n" RESET);
                    i++;
                    break;
                }
            }
            install_software(dependency_instructions[i], true);

            i++;
            if (i >= 500) {
                printf("Warning: Loop max use reached (500).\n");
                break;
            }
        }
    }

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
            printf(RED "Error at build instructions numero %d for %s\n" RESET,
                   i, package_name);

            set_exit(1);
            break;
        }

        if (is_debug())
            printf("Success at executing %s at build step.\n",
                   build_instructions[i]);

        i++;
        if (i >= 500) {
            printf("Warning: Loop max use reached (500).\n");
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
            printf("Error at install instructions numero %d for %s\n", i,
                   package_name);

            set_exit(1);
            break;
        }
        if (is_debug())
            printf("Success at executing %s at install step.\n",
                   install_instructions[i]);

        i++;
        if (i >= 500) {
            printf("Warning: Loop max use reached (500).\n");
            break;
        }
    }

    FILE *read_user_software_db = fopen("/etc/cydramanager.d/usdb", "r");
    FILE *write_user_software_db = fopen("/etc/cydramanager.d/temp", "w");
    char read[512];
    char read_buffer[512];
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "%s ", package_name);

    while (fgets(read, sizeof(read), read_user_software_db)) {
        strcpy(read_buffer, read);

        if (strlen(space_clean(read_buffer)) <= 0) {
            continue;
        }

        if (strstr(read, pattern)) {
            continue;
        }

        fputs(read, write_user_software_db);
    }

    char new_line[512];
    snprintf(new_line, sizeof(new_line), "%s %s\n", package_name,
             package_version);
    fputs(new_line, write_user_software_db);

    fclose(read_user_software_db);
    fclose(write_user_software_db);

    rename("/etc/cydramanager.d/usdb", "/etc/cydramanager.d/usdb_old");
    rename("/etc/cydramanager.d/temp", "/etc/cydramanager.d/usdb");

    printf("Installation for the package %s is done.\n", package_name);

    return true;
}
