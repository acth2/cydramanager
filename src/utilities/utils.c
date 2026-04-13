#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

char *space_clean(char *str) {
    if (!str)
        return str;
    char *read = str, *write = str;

    while (*read) {
        if (!isspace((unsigned char)*read)) {
            *write++ = *read;
        }
        read++;
    }
    *write = '\0';
    return str;
}

void replace_proc(char *str, char *replace) {
    char buffer[1024];
    char *pos;

    while ((pos = strstr(str, "/#PROC#/")) != NULL) {
        buffer[0] = '\0';

        strncat(buffer, str, pos - str);

        strcat(buffer, replace);

        strcat(buffer, pos + 8);
        strcpy(str, buffer);
    }
}

int is_empty(FILE *f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    return size == 0;
}

int is_number(const char *str) {
    if (*str == '\0') return 0;

    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

long long get_dir_size(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return 0;

    struct dirent *entry;
    long long total = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == -1)
            continue;

        if (S_ISDIR(st.st_mode)) {
            total += get_dir_size(fullpath);
        } else {
            total += st.st_size;
        }
    }

    closedir(dir);
    return total;
}
