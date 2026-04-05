#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

void print_help() {
    printf("cydramanager - cydra package manager\n");
    printf("\nUsage:\n");
    printf("  cydramanager [command] [arguments]\n");
    printf("\nCommands:\n");
    printf("  help       Show this help message\n");
    printf("  update     Update the system\n");
    printf("  install    Install a package in the system\n");
    printf("  version    Show cydramanager version\n");
};

void print_version() { 
    printf("cydramanager version 0.1.0\n");
}

#endif
