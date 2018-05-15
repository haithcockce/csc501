#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

struct sigaction sa;
void handle_signal(int signo);

/*
 * When signals handlers are registered, you can either register just the 
 * sa_handler or sa_sigaction. sa_handler is more simple but setting sa_flags
 * with SA_SIGINFO causes a ton more info to be passed to the handler and 
 * sa_sigaction needs to be set instead of sa_handler. See sigaction(2) for
 * more details on this
 */
void handle_signal(int signo){
    printf("Received %d\n", signo);
}



void main() {
    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sleep(100);

}
