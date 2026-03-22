#include <stdio.h>
#include <stdlib.h>
// #include <string.h>

#include "arguments/arg.h"
#include "main.h"
#include "update/update_manager.h"

int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            switch (arg2enum(argv[i])) {
            case ARG_HELP: {
                print_help();
                break;
            }

            case ARG_VERSION: {
                print_version();
                break;
            }

            case ARG_UPDATE: {
                SoftwareDB db = get_software_database();
                free(db.software_counter);
                free(db.software_map);
                break;
            }

            case ARG_UNK: {
                printf("unknown argument %s, use --help to see available "
                       "options.\n",
                       argv[i]);
            }
            default:
                break;
            }
        }
        return 0;
    }
    printf("Use --help to see available options.\n");

    return 0;
}
