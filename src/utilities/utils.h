#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GRAY    "\033[90m"
#define WHITE   "\033[37m"
#define RESET   "\033[0m"

char *space_clean(char *str);
void replace_proc(char *str, char* replace);
int is_number(const char *str);
int is_empty(FILE *f);

#endif
