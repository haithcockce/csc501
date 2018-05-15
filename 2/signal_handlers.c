#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include "signal_handlers.h"


void setup_signal_handling() {
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, handle_sigint);
    sigemptyset(&curr_sigmask);
    sigemptyset(&prev_sigmask);
    sigprocmask(0, NULL, &curr_sigmask);
    sigprocmask(0, NULL, &prev_sigmask);
    //sa.sa_handler = handle_sigint
}

void setup_sigmask() {
    sigprocmask(SIG_UNBLOCK, &curr_sigmask, &prev_sigmask); 
}

void restore_sigmask() {
    sigprocmask(SIG_BLOCK, &prev_sigmask, NULL);
    sigprocmask(0, NULL, &curr_sigmask);
}

void handle_sigint(int sig) {
    //printf("%d CAUGHT SIGINT. PARENT IS %d\n", getpid(), getppid());
    return;
}
