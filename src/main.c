#include "main.h"
#include "arguments/arg.h"
#include "arguments/debug/debug.h"
#include "configuration/configuration.h"
#include "exit/exit.h"
#include "install/install_manager.h"
#include "src/remove/remove_manager.h"
#include "update/update_manager.h"
#include "utilities/utils.h"
#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
            char *default_args = getDefaultArg();
            if (strlen(default_args) > 0) {
                char args[128]        [512];
                int i = 0;

                char *token = strtok(default_args, " ");

                while (token != NULL) {
                    strcpy(args[i], token);
                    i++;
                    token = strtok(NULL, " ");
                }

                for (int j = 0; j < i; j++) {
                    if (arg2enum(args[j]) == ARG_DEBUG) {
                        set_debug(true);
                    }
                }
            }

            if (arg2enum(argv[i]) == ARG_DEBUG) {
                set_debug(true);
            }

            if (arg2enum(argv[i]) == ARG_CONF) {
                if (i + 1 < argc) {
                    change_configuration(argv[i + 1]);
                    check_crash();

                    i++;
                } else {
                    printf(RED "Error: the argument %s need to provide a "
                               "path.\n" RESET,
                           argv[i]);
                }
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
    printf(YELLOW "   -debug     " RESET "Show detailed informations\n");
    printf(YELLOW "   -conf      " RESET "Set an explicit configuration file\n");
};

void print_version() { printf(RESET "cydramanager" YELLOW " 1.1.0\n" RESET); }

void check_crash() {
    if (did_crash()) {
        curl_global_cleanup();
        exit(get_exit());
    }
}

