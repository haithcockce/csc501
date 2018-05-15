#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main() {
    char* path = getenv("PATH");
    printf("%s\n\n\n", path);

    // It appears that getenv does not return the
    // variable requested in the path thankfully. 

    char* next_path; 
    char* command_name = "useradd";
    char absolute_pathname[512];
    next_path = strtok(path, ":");
    while(next_path != NULL) {
        printf("str: %s\n", next_path);
        strcpy(absolute_pathname, next_path);
        strcat(absolute_pathname, "/");
        strcat(absolute_pathname, command_name);
        printf("abs: %s\n\n", absolute_pathname);
        next_path = strtok(NULL, ":");
    }
    if(access("/sbin/useradd", F_OK) == 0) {
        printf("Could access\n");
    }
    else {
        printf("Couldn't access\n");
    }

}    
