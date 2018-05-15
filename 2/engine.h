#define TRUE    1
#define FALSE   0

#define LOGOUT      -10
#define ENDINPUT    -20
#define EMPTYINPUT  -30
#define EFORKFAIL   -1
#define EXEC_SUCC   0
#define EEXEC       1
#define PIPEIN_FILE "pipein"
#define PIPEOUT_FILE "pipeout"
#define F_IN     "input"
#define F_OUT    "output"
#define F_ERR    "error"

/**
 * STDIN_FILENO is 0
 * STDOUT_FILENO is 1 
 * STDERR_FILENO is 2 
 * from unistd.h
 */
typedef struct cmd_t Command;

int fd_i, fd_o, flags;

void setup_io_redirect(Command* command);
void __swap_pipe(Command* command);
void __open_input(Command* command);
void __clean_files();
void __handle_execvp_failue(Command* command);
int run(Pipe p);

int handle_write(Pipe p);
void __setup();
