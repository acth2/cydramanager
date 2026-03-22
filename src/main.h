#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>

void print_help() {
    printf("cydramanager - Simple package manager\n");
    printf("\nUsage:\n");
    printf("  cydramanager [command]\n");
    printf("\nCommands:\n");
    printf("  help       Show this help message\n");
    printf("  update     Update the system\n");
    printf("  version    Show cydramanager version\n");
    exit(0);
};

void print_version() { 
    printf("cydramanager version 0.1.0\n");
    exit(0);
}

#endif
