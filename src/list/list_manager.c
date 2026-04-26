#include "list_manager.h"
#include "../utilities/utils.h"
#include "src/exit/exit.h"
#include <stdio.h>
#include <string.h>

void print_installed_softwares() {
    FILE *user_database = fopen("/etc/cydramanager.d/usdb", "r");
    if (!user_database) {
        printf(RED
               "Error: The user software database file is not found.\n" RESET);
        set_exit(1);
    }
    printf(GRAY "-> Getting the database informations\n" RESET);

    char buffer[1024];
    int i = 0;
    while (fgets(buffer, sizeof(buffer), user_database)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        i++;
        printf("%d - %s\n", i, buffer);
    }
    fclose(user_database);

    printf(YELLOW "\nThere is total of %d softwares installed on your system.\n" RESET, i);
}
