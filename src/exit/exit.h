#ifndef EXIT_H
#define EXIT_H

#include <stdbool.h>

bool did_crash(void);
void set_exit(int code);
int get_exit(void);

#endif
