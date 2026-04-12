#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

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
