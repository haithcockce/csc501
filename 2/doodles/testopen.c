#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void main() {
    struct stat stats; 
    stat("../../1/makefile", &stats);
    printf("Mode: %lo\n", (unsigned long) stats.st_mode);
    /*
    int fd_rel = open("rel", (O_WRONLY | O_CREAT | O_TRUNC), 0755); // This is equivalent to creat();
    int fd_currdir = open("../../1/makefile", (O_WRONLY | O_CREAT | O_TRUNC), 0755);
    int fd_abs = open("/home/dev/School/501/2/doodles/abs", (O_WRONLY | O_CREAT | O_TRUNC), 0755);

    char buff_rel[10] = "relative\n";
    char buff_cd[19] = "current directory\n";
    char buff_abs[11] = "absolute \n";

    write(fd_rel, buff_rel, 10);
    write(fd_currdir, buff_cd, 19);
    write(fd_abs, buff_abs, 11);

    close(fd_rel);
    close(fd_currdir);
    close(fd_abs);
*/

    /**
     * testing dup2
     */
/*
    char buff_out[22] = "\n\nWriting to STDOUT\n\n";
    write(STDOUT_FILENO, buff_out, 22);

    int fd_test = open("redirect", (O_WRONLY | O_CREAT | O_TRUNC), 0755);
    dup2(fd_test, STDOUT_FILENO);
    printf(buff_out);
    close(fd_test);
    printf(buff_out);
    */
    // So the original file descriptor is indeed closed, but the duplicated over fd is gone and the 
    // original fd is still there. 
}
