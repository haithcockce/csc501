int __cd(Command *command);
int __echo(Command *command);
int __logout();
int __nice(Command *command);
int __pwd();
int __setenv(Command *command);
int __unsetenv(Command *command);
int __where(Command *command);
int __run_builtin(Command *command);
int __is_builtin(char* command);

extern char** environ;
