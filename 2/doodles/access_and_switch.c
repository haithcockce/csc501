#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

void main() {
    if(access("herpaderpa.txt", F_OK) != 0)
        printf("COULD NOT FIND FILE\n");
    else { 
        printf("FOUND FILE\n");
        rename("herpaderpa.txt", "notarock.txt");
    }
}
