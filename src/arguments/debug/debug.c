#include "debug.h"

static bool DEBUG = false;

bool is_debug(void) {
    return DEBUG;
}

void set_debug(bool state) {
    DEBUG = state;
}
