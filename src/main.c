#include "main.h"
#include "arguments/arg.h"
#include "arguments/debug/debug.h"
#include "install/install_manager.h"
#include "update/update_manager.h"
#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_SSL);

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (arg2enum(argv[i]) == ARG_DEBUG) {
                set_debug(true);
            }
        }

        for (int i = 1; i < argc; i++) {
            switch (arg2enum(argv[i])) {
            case HELP: {
                print_help();
                break;
            }

            case VERSION: {
                print_version();
                break;
            }

            case UPDATE: {
                SoftwareDB db = get_current_database("/etc/cydramanager.d/sdb");
                UpdatedDB udb = get_updated_database(db);
                if (!apply_software_db(db)) {
                    printf(
                        "Error: The database could not have been updated.\n");
                    break;
                }

                for (int i = 0; i < udb.outdated_size; i++) {
                    int index = udb.outdated_index[i];
                    update_package(udb, index, i <= 0 ? false : true);
                }

                free(udb.updated_db.software_map);
                free(udb.outdated_index);
                break;
            }

            case INSTALL: {
                if(i + 1 < argc) {
                    char *package_name = argv[i + 1];
                    install_software(package_name, false);

                    i++;
                } else {
                    printf("Error: you need to provide a package name in order to install it.\n");
                }
                break;
            }

            case UNINSTALL: {
                break;
            }

            case UNK: {
                printf("unknown argument %s, use --help to see available "
                       "options.\n",
                       argv[i]);
                break;
            }
            default:
                break;
            }
        }
        curl_global_cleanup();
        return 0;
    }
    printf("Use --help to see available options.\n");
    return 0;
}
