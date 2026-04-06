#include "install_manager.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

bool install_software(char *package_name) {
    if (strlen(package_name) <= 0) {
        printf("Error: you need to provide a package name in order to install "
               "it.\n");
        return false;
    }

    char *cache_dir = "/tmp/cydramanager.tmp";

    system("rm -rf /tmp/cydramanager.tmp");
    if (mkdir(cache_dir, 0777) == -1) {
        printf(
            "Error: could not create a temporary directory in /tmp folder.\n");
        return false;
    }

    char *mirror_link = "https://raw.githubusercontent.com/acth2/"
                        "cydramanager-db/refs/heads/main/softwares/";
    char package_link[1024] = "";

    snprintf(package_link, sizeof(package_link), "%s%s", mirror_link,
             package_name);

    char package_path[512] = "/tmp/cydramanager.tmp/";
    char package_instructions_path[512] = "/tmp/cydramanager.tmp/instructions";
    strcat(package_path, package_name);

    CURL *curl = curl_easy_init();
    FILE *file = fopen(package_path, "wb");

    if (!curl || !file) {
        printf("Error: Unexpected behaviour during the installation of the "
               "software %s\n",
               package_name);
        return false;
    }
    printf("-> Getting the informations of the package %s from the current mirror.\n", package_name);

    curl_easy_setopt(curl, CURLOPT_URL, package_link);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode cperf = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (cperf == CURLE_COULDNT_CONNECT) {
        printf("Error: You are not connected to the internet, the installation "
               "cannot happen.\n");
        return false;
    }

    if (cperf != CURLE_OK) {
        printf("Error: an unexpected error occured.\n");
        return false;
    }
    fclose(file);

    char package_version[256];
    FILE *package_version_file = fopen(package_path, "r");
    fgets(package_version, 256, package_version_file);
    fclose(package_version_file);
    package_version[strcspn(package_version, "\n")] = '\0';

    if (strcmp(package_version, "404: Not Found") == 0) {
        printf("Error: The package you asked to install does not exist in the "
               "current mirror.\n");
        return false;
    }

    printf("## Package %s, version %s found.\n", package_name, package_version);
    FILE *instructions_file = fopen(package_instructions_path, "wb");
    CURL *instructions_curl = curl_easy_init();
    char instructions_link[512] = "";
    strcpy(instructions_link, mirror_link);
    strcat(instructions_link, "instructions/");
    strcat(instructions_link, package_name);

    if (!instructions_curl || !instructions_file) {
        printf("Error: Unexpected behaviour during the installation of the software %s\n", package_name);
        return false;
    }

    curl_easy_setopt(instructions_curl, CURLOPT_URL, instructions_link);

    curl_easy_setopt(instructions_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(instructions_curl, CURLOPT_WRITEDATA, instructions_file);

    CURLcode cperf_instructions = curl_easy_perform(instructions_curl);
    curl_easy_cleanup(instructions_curl);

    if (cperf_instructions == CURLE_COULDNT_CONNECT) {
        printf("Error: You are not connected to the internet, the installation cannot happen.\n");
        return false;
    }

    if (cperf_instructions != CURLE_OK) {
        printf("Error: An unexpected error occured.\n");
        return false;
    }
    fclose(instructions_file);

    return true;
}
