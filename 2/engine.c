/* Engine.c
 *
 * The part of the shell that actually runs the commands provided.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "parse.h"
#include "engine.h"
#include "builtins.h"
#include "signal_handlers.h"

//int fd_i, fd_o, flags;

int run(Pipe pipe) {
    //If we reached the end of the pipe pieces or no command is given, return success
    if(pipe == NULL || pipe->head == NULL || pipe->head->args[0] == NULL) {
        return 0;
    }

    //printf("DEBUG: run: starting to run this pipe\n");
    Command *command;
    pid_t cpid;
    int status;

    // For each command in this pipe piece...
    for(command = pipe->head; command != NULL; command = command->next) {
       
        // Firsh check if we are logging out now and logout if so
        if(strcmp(command->args[0], "logout") == 0) {
            return LOGOUT;
        } 
        else if(strcmp(command->args[0], "end") == 0) {
            return ENDINPUT;
        }
        else if(strcmp(command->args[0], "empty") == 0) {
            return EMPTYINPUT;
        }

        fflush(stdout);
        fflush(stderr);

        // Check if the command is a built in command running it
        // if so and continuing to the next command if so 
        if(command->out == Tnil && __is_builtin(command->args[0])) {
            //printf("DEBUG: run: last command and am builtin\n");
            int in_bak = dup(STDIN_FILENO);
            int out_bak = dup(STDOUT_FILENO);
            int err_bak = dup(STDERR_FILENO);
            setup_io_redirect(command);
            int result = __run_builtin(command);
            dup2(in_bak, STDIN_FILENO);
            dup2(out_bak, STDOUT_FILENO);
            dup2(err_bak, STDERR_FILENO);
            close(in_bak);
            close(out_bak);
            close(err_bak);
            if(result == LOGOUT) {
                return LOGOUT;
            }
            continue;
        }
        
        // Then attempt to fork exiting on failure
        cpid = fork();
        if(cpid == -1) {
            //printf("DEBUG: run: failed to fork\n");
            return EFORKFAIL;
        }

        // If we are the parent...
        if(cpid != 0) {
            //printf("DEBUG: run: I am the parent, waiting on child %d\n", cpid);
            //wait on the child to finish and go to the next command

            wait(&status);
            if(WEXITSTATUS(status) == EEXEC) {
                return EEXEC;
            }
            // YOU MAY NEED TO CHANGE THIS TO HANDLE REDIRECTION
            //printf("DEBUG: run: I am the parent and a child exited\n");
            continue;
        }

        // Otherwise we are the child.
        // Go ahead and setup the file descriptors as necessary
        setup_io_redirect(command);

        if(__is_builtin(command->args[0])) {
            //printf("DEBUG: run: command is builtin and running in subshell\n");
            if(__run_builtin(command) == EEXEC) {
                exit(EEXEC);               
            }
            exit(EXEC_SUCC);
        }
        //printf("DEBUG: run: it was not builtin\n");
        // Note that we will not reach here if it is a built in function since builtin()
        // will exit() and thus terminate completely
        if(execvp(command->args[0], command->args) == -1) {
            __handle_execvp_failue(command);
            exit(EEXEC);
        }
        
    }
    return run(pipe->next);
}


/**
 * setup_io_redirect
 *
 * DESC
 *
 * ARGS
 *
 * RETN
 *
 * MODS
 *
 */
void setup_io_redirect(Command* command) {

    // Let's make sure the buffers are clear before we start writing
    fflush(stdin);
    fflush(stdout);
    fflush(stderr);
    //setbuf(stdout, NULL);

    __swap_pipe(command);

    // First check through input redirect
    __open_input(command);

    // Now go through all the possible output options

    flags = O_WRONLY | O_CREAT;
    if(command->out == Tout || command->out == ToutErr ||
            command->out == Tpipe || command->out == TpipeErr) {
        // > and >& need to truncate the output file
        // Let's truncate the pipe file as well
        flags |= O_TRUNC;
    }
    if(command->out == Tout || command->out == Tapp || 
            command->out == Tpipe || command->out == TpipeErr ||
            command->out == ToutErr || command->out == TappErr) {
        // For any >, >&, >>, >>&, |, |&, you will need to open a file to redirect to
        //printf("DEBUG: setup_io_redirect: Filename to write out to: %s\n", command->outfile);

        // If we are writing to | or |&, then write to our pipe file
        if(command->out == Tpipe || command->out == TpipeErr) {
            fd_o = open(PIPEOUT_FILE, flags, 0755);       
        }

        // Else we are writing to >, >>, >&, or >>&
        else {
            fd_o = open(command->outfile, flags, 0755);
        }

        // Sanity check. Make sure to open at the end of the file. 
        lseek(fd_o, 0, SEEK_END);
        dup2(fd_o, STDOUT_FILENO);

        // >& and >>& will need to have stderr modified to point to the same thing as stdout
        if(command->out == ToutErr || command->out == TappErr || command->out == TpipeErr) {
            dup2(fd_o, STDERR_FILENO);
        }
        close(fd_o);
    }       
}


/**
 * __swap_pipe
 *
 * DESC
 *
 * ARGS
 *
 * RETN
 *
 * MODS
 *
 */
void __swap_pipe(Command* command) {
    if(command->in == Tin || command->in == Tpipe || command->in == TpipeErr) {
        if(access(PIPEOUT_FILE, F_OK) == 0) {
            remove(PIPEIN_FILE);
            rename(PIPEOUT_FILE, PIPEIN_FILE);
        }
    }
}

/**
 * __open_input
 *
 * DESC
 *
 * ARGS
 *
 * RETN
 *
 * MODS
 *
 */
void __open_input(Command* command) {
    if(command->in == Tin || command->in == Tpipe || command->in == TpipeErr) {
        if(command->in == Tpipe || command->in == TpipeErr) {
            fd_i = open(PIPEIN_FILE, O_RDONLY, 0755);
        }
        else {
            fd_i = open(command->infile, O_RDONLY, 0755);
        }
        dup2(fd_i, STDIN_FILENO);
        close(fd_i);
    }
}

/**
 * __clean_files
 *
 * DESC
 *
 * ARGS
 *
 * RETN
 *
 * MODS
 *
 */
void __clean_files() {
    if(access(PIPEIN_FILE, F_OK) == 0)
        remove(PIPEIN_FILE);
    if(access(PIPEOUT_FILE, F_OK) == 0)
        remove(PIPEOUT_FILE);
}


/**
 * __handle_execvp_failure
 *
 * DESC
 *
 * ARGS
 *
 * RETN
 *
 * MODS
 *
 */
void __handle_execvp_failue(Command* command) {
    if(errno == ENOENT) {
        printf("%s: Command not found\n", command->args[0]);
    }
    if(errno == EACCES) {
        printf("%s: Permission denied\n", command->args[0]);
    }   
}

/**
 * __setup
 *
 * DESC
 *
 * ARGS
 *
 * RETN
 *
 * MODS
 *
 */
void __setup() {
    setup_signal_handling();

}
