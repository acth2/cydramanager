#include "exit.h"
#include <stdbool.h>

static int EXIT = 0;

bool did_crash(void) {
        return EXIT != 0;
}

void set_exit(int code) {
        EXIT = code;
}

int get_exit(void) {
        return EXIT;
}
