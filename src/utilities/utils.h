#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <string.h>

static char* space_clean(char* str) {
    if (!str) return str;
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

static void replace_proc(char *str) {
    char buffer[1024];
    char *pos;
    
    while ((pos = strstr(str, "/#PROC#/")) != NULL) {
        buffer[0] = '\0';

        strncat(buffer, str, pos - str);

        strcat(buffer, "4");

        strcat(buffer, pos + 8);
        strcpy(str, buffer);
    }
}

#endif
