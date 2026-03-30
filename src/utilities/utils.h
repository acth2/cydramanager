#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>

char* space_clean(char* str) {
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


#endif
