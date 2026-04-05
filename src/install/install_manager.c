#include "install_manager.h"
#include "src/utilities/software.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

SoftwareDB get_deployable_softwares() {
    SoftwareDB db = {0};
    char *cache_dir = "/tmp/cydramanager.tmp";

    system("rm -rf /tmp/cydramanager.tmp");
    if (mkdir(cache_dir, 0777) == -1) {
        printf(
            "Error: could not create a temporary directory in /tmp folder.\n");
        return db;
    }

    char *mirror_link = "https://raw.githubusercontent.com/acth2/cydramanager-db/refs/heads/main/softwares/";
    

    return db;
}
