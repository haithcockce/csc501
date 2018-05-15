
sigset_t curr_sigmask; 
sigset_t prev_sigmask;
//struct sigaction sa;

void setup_signal_handling();
void handle_sigint();
