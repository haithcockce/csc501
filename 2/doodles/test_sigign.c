#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void main() {
    signal(SIGQUIT, SIG_IGN);
    sleep(1000);
}
