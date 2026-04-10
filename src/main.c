#include "main.h"
#include "exit/exit.h"
#include "arguments/arg.h"
#include "arguments/debug/debug.h"
#include "install/install_manager.h"
#include "src/remove/remove_manager.h"
#include "update/update_manager.h"
#include "utilities/utils.h"
#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (geteuid() != 0) {
        printf(RED "Error: cydramanager needs to be run with elevated "
                   "privileges.\n" RESET);
        return 1;
    }

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

                check_crash();

                if (!apply_software_db(db)) {
                    printf(RED "Error: The database could not have been "
                               "updated.\n" RESET);
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
                if (i + 1 < argc) {
                    char *package_name = argv[i + 1];
                    install_software(package_name, false);

                    check_crash();

                    i++;
                } else {
                    printf(RED
                           "Error: you need to provide a package name in order "
                           "to install it.\n" RESET);
                }
                break;
            }

            case UNINSTALL: {
                if (i + 1 < argc) {
                    char *package_name = argv[i + 1];
                    remove_software(package_name);

                    check_crash();

                    i++;
                } else {
                    printf(RED
                           "Error: you need to provide a package name in order "
                           "to remove it.\n" RESET);
                }
                break;
            }

            case UNK: {
                printf("unknown command %s, use --help to see available "
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

void print_help() {
    printf(RESET "cydramanager - cydra package manager\n");
    printf(RESET "\nUsage:\n");
    printf(RESET "   cydramanager" YELLOW " [command] " RESET "[arguments]\n");
    printf(RESET "\nCommands:\n");
    printf(YELLOW "   help       " RESET "Show this help message\n");
    printf(YELLOW "   update     " RESET "Update the system\n");
    printf(YELLOW "   install    " RESET "Install a package in the system\n");
    printf(YELLOW "   remove     " RESET "Remove a package in the system\n");
    printf(YELLOW "   version    " RESET "Show cydramanager version\n");
    printf(RESET "\nArguments:\n");
    printf(YELLOW "   -debug     " RESET "Show detailed build steps and logs\n");
};

void print_version() { printf(RESET "cydramanager" YELLOW " 0.1.0\n"); }

void check_crash() {
    if (did_crash()) {
        curl_global_cleanup();
        exit(get_exit());
    }
}
