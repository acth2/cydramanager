#include <stdbool.h>
#include <stdio.h>
#include "arguments/arg.h"
#include "main.h"
#include "update/update_manager.h"
#include <curl/curl.h>
#include <stdlib.h>

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
                curl_global_init(CURL_GLOBAL_SSL);

                SoftwareDB db = get_current_database("/etc/cydramanager.d/sdb");
                UpdatedDB udb = get_updated_database(db);
                if (!apply_software_db(db)) {
                   printf("Error: The database could not have been updated.\n");
                   break;
                }

                for (int i = 0; i < udb.outdated_size; i++) {
                    int index = udb.outdated_index[i];
                    update_package(udb, index, false);
                }
                               
                free(udb.updated_db.software_map);
                free(udb.outdated_index);

                curl_global_cleanup();
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
