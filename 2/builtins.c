#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For getcwd
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include "parse.h"
#include "engine.h"
#include "builtins.h"

/* builtin
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __run_builtin(Command *command) {
    char *command_name = command->args[0];

    if(strcmp(command_name, "cd") == 0) {
        __cd(command);
    }
    else if (strcmp(command_name, "echo") == 0) {
        __echo(command);
    }
    else if(strcmp(command_name, "logout") == 0) {
        return __logout();
    }
    else if(strcmp(command_name, "nice") == 0) {
        return __nice(command);
    }
    else if(strcmp(command_name, "pwd") == 0) {
        __pwd();
    }
    else if(strcmp(command_name, "setenv") == 0) {
        __setenv(command);
    }
    else if(strcmp(command_name, "unsetenv") == 0) {
        __unsetenv(command);
    }
    else if(strcmp(command_name, "where") == 0) {
        return __where(command);
    }
    else {
        return FALSE;
    }

    // If we fell out of the if/then lattice, then we ran something and need to leave. 
    // This will send SIGCHLD to the parent to allow wait() to finish up for the parent
    return TRUE;
}

/* __cd
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __cd(Command *command) {
    if(command->args[1] != NULL) {
        chdir(command->args[1]);
    }
    return 0;
}

/* __echo
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __echo(Command *command) {
    if(command->args[1] != NULL) {
        printf(command->args[1]);
    }
    printf("\n");
    return 0;
}

/* __logout
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __logout() {
    return LOGOUT;
}

/* __nice
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __nice(Command *command) {
    int prio = 4;

    // 'host% nice'
    if(command->nargs == 1) {
        setpriority(PRIO_PROCESS, 0, prio);
        return EXEC_SUCC;
    }
    
    // Let's check the next token for a nice value or command
    char* token = command->args[1];
    char c = token[0];
    int gave_prio = FALSE;

    // If the prio is given
    if(isdigit(c) || ((c == '-' || c == '+') && isdigit(token[1]))) {
        prio = atoi(token);
        gave_prio = TRUE;

        // 'host% nice ##'
        if(command->nargs == 2) { 
            setpriority(PRIO_PROCESS, 0, prio);
            return EXEC_SUCC;
        }
    }

    // Reaching here means either of the following were ran: 
    // 'host% nice [COMMAND]
    // 'host% nice [+|-][NICE] [COMMAND]
    // and for the last case we already have the prio. 

    // Grab a pointer to the command to run and reduce the amount of args we have.
    // We need a semi-shallow copy of the command to execute with but the original 
    // will need to be freed later
    Command* niced_command = (Command*) malloc(sizeof(Command));
    memcpy(niced_command, command, sizeof(Command));

    // Shift over the reference to the args and decrement arg count since we
    // already grabbed the prio
    niced_command->nargs = gave_prio ? niced_command->nargs - 2 : niced_command->nargs - 1;
    niced_command->args = gave_prio ? niced_command->args + 2 : niced_command->args + 1;

    // grab the original prio to switch back and forth
    int original_prio = getpriority(PRIO_PROCESS, 0);

    // If the niced command is built in and the last command in the pipe
    if(niced_command->out == Tnil && __is_builtin(niced_command->args[0])) {
        //printf("DEBUG: run: last command and am builtin\n");
        // backup the STD* files and redirect IO
        int in_bak = dup(STDIN_FILENO);
        int out_bak = dup(STDOUT_FILENO);
        int err_bak = dup(STDERR_FILENO);
        setup_io_redirect(niced_command);

        // Set prio, run the command, then reset prio
        setpriority(PRIO_PROCESS, 0, prio);
        int result = __run_builtin(niced_command);
        setpriority(PRIO_PROCESS, 0, original_prio);

        // restore STD files and exit
        dup2(in_bak, STDIN_FILENO);
        dup2(out_bak, STDOUT_FILENO);
        dup2(err_bak, STDERR_FILENO);
        close(in_bak);
        close(out_bak);
        close(err_bak);
        if(result == LOGOUT) {
            return LOGOUT;
        }
        return EXEC_SUCC;
    }

    // If not a built in, Then attempt to fork exiting on failure
    int status;
    pid_t cpid = fork();
    if(cpid == -1) {
        //printf("DEBUG: run: failed to fork\n");
        return EFORKFAIL;
    }

    // If we are the parent...
    if(cpid != 0) {
    //printf("DEBUG: run: I am the parent, waiting on child %d\n", cpid);

        //wait on the child to finish and go to the next command
        wait(&status);

        // If our child failed to execute, catch it so we stop the pipe
        if(WEXITSTATUS(status) == EEXEC) {
            return EEXEC;
        }
        //printf("DEBUG: run: I am the parent and a child exited\n");
        return EXEC_SUCC;
    }

    // Otherwise we are the child.
    // Go ahead and setup the file descriptors as necessary
    setup_io_redirect(niced_command);

    setpriority(PRIO_PROCESS, 0, prio);
    if(__is_builtin(niced_command->args[0])) {
        //printf("DEBUG: run: command is builtin and running in subshell\n");
        int exec_result = __run_builtin(niced_command);
        exit(exec_result);
    }
    //printf("DEBUG: run: it was not builtin\n");
    // Note that we will not reach here if it is a built in function since builtin()
    // will exit() and thus terminate completely
    if(execvp(niced_command->args[0], niced_command->args) == -1) {
        __handle_execvp_failue(niced_command);
        exit(EEXEC);
    }        
    return EXEC_SUCC;
}

/* __pwd
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __pwd() {
    char *cwd = (char*) malloc(1024);
    getcwd(cwd, 1024);
    printf("%s\n", cwd);
    free(cwd);
    return 0;
}

/* __setenv
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __setenv(Command *command) {
    // If 'setenv' was called without parameters...
    if(command->nargs == 1) {
        unsigned int i = 0;
        while(environ[i] != NULL) {
            printf("%s\n", environ[i]);
            fflush(stdout);
            fflush(stderr);
            i++;
        }
    }

    // If setenv <var> was called...
    else if(command->nargs == 2 && command->args[1] != NULL) {
        setenv(command->args[1], '\0', TRUE);
    }
    else if(command->nargs == 3 && command->args[1] != NULL && command->args[2] != NULL) {
        setenv(command->args[1], command->args[2], 1);
    }
    return 0;
}

/* __unsetenv
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __unsetenv(Command *command) {
    if(command->nargs == 2 && command->args[1] != NULL) {
        unsetenv(command->args[1]);
    }
    return 0;
}

/* __where
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __where(Command *command) {

    if(command->nargs == 1) {
        printf("where: Too few arguments\n");
        return EXEC_SUCC;
    }

    // Grab the value of PATH and the command
    // We have to copy the value of PATH be cause getenv returns only a pointer
    // to the same value. Tokenizing it via strtok causes the pointer to the
    // environment variable to increment to blowing away its value.
    char path_env[512];
    strcpy(path_env, getenv("PATH"));
    char* command_name = command->args[1];

    // First check if it is a builtin
    if(__is_builtin(command->args[1])) {
        printf("%s is a shell built-in\n", command_name);
    }

    // Now check if it is in .ushrc


    // Now check through all the PATH directories
    // Begin tokenizing it based on ':' as the delimiter
    char* next_path;
    char absolute_pathname[512];
    next_path = strtok(path_env, ":");
    while(next_path != NULL) {
        strcpy(absolute_pathname, next_path);
        strcat(absolute_pathname, "/");
        strcat(absolute_pathname, command_name);
        if(access(absolute_pathname, F_OK) == 0) {
            printf("%s\n", absolute_pathname);
        }
        next_path = strtok(NULL, ":");
    }
    return EXEC_SUCC;
}

/* __is_builtin
 *
 * DESC:
 *
 * PARM:
 *
 * RETN:
 *
 * MODS:
 *
 */
int __is_builtin(char* command) {
    //printf("DEBUG: __is_builtin: checking builtin\n");
    char *c = command;

    return (
       (strcmp(c, "cd") == 0) ||
       (strcmp(c, "echo") == 0) || 
       (strcmp(c, "logout") == 0) || 
       (strcmp(c, "nice") == 0) || 
       (strcmp(c, "pwd") == 0) || 
       (strcmp(c, "setenv") == 0) || 
       (strcmp(c, "unsetenv") == 0) || 
       (strcmp(c, "where") == 0)
    );
}
