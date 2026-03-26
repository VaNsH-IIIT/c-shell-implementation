#include <shell.h>
// ############## LLM Generated Code Begins ################ 

void parse_command_line(char *line, pipeline_t *pipeline) {
    pipeline->num_commands = 0;
    pipeline->commands = malloc(MAX_PIPES * sizeof(command_t));
    
    // Split by pipes first
    char *pipe_token;
    char *pipe_saveptr;
    char *line_copy = strdup(line);
    
    pipe_token = strtok_r(line_copy, "|", &pipe_saveptr);
    
    while (pipe_token && pipeline->num_commands < MAX_PIPES) {
        command_t *cmd = &pipeline->commands[pipeline->num_commands];
        cmd->input_file = NULL;
        cmd->output_file = NULL;
        cmd->append_output = 0;
        
        // Parse redirections and arguments
        char *token;
        char *saveptr;
        char *cmd_copy = strdup(pipe_token);
        char *args_str = malloc(MAX_INPUT_SIZE);
        args_str[0] = '\0';
        
        token = strtok_r(cmd_copy, " \t", &saveptr);
        
        while (token) {
            if (strcmp(token, "<") == 0) {
                // Input redirection
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    if (cmd->input_file) free(cmd->input_file);
                    cmd->input_file = strdup(token);
                }
            } else if (strcmp(token, ">") == 0) {
                // Output redirection (overwrite)
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    if (cmd->output_file) free(cmd->output_file);
                    cmd->output_file = strdup(token);
                    cmd->append_output = 0;
                }
            } else if (strcmp(token, ">>") == 0) {
                // Output redirection (append)
                token = strtok_r(NULL, " \t", &saveptr);
                if (token) {
                    if (cmd->output_file) free(cmd->output_file);
                    cmd->output_file = strdup(token);
                    cmd->append_output = 1;
                }
            } else {
                // Regular argument
                if (strlen(args_str) > 0) {
                    strcat(args_str, " ");
                }
                strcat(args_str, token);
            }
            token = strtok_r(NULL, " \t", &saveptr);
        }
        
        // Parse arguments
        int argc;
        cmd->args = tokenize_command(args_str, &argc);
        
        free(cmd_copy);
        free(args_str);
        pipeline->num_commands++;
        pipe_token = strtok_r(NULL, "|", &pipe_saveptr);
    }
    
    free(line_copy);
}

char **tokenize_command(char *cmd_str, int *argc) {
    char **args = malloc(MAX_ARGS * sizeof(char*));
    *argc = 0;
    
    char *token;
    char *saveptr;
    char *cmd_copy = strdup(cmd_str);
    
    token = strtok_r(cmd_copy, " \t", &saveptr);
    while (token && *argc < MAX_ARGS - 1) {
        args[*argc] = strdup(token);
        (*argc)++;
        token = strtok_r(NULL, " \t", &saveptr);
    }
    args[*argc] = NULL;
    
    free(cmd_copy);
    return args;
}

void execute_pipeline(pipeline_t *pipeline) {
    if (pipeline->num_commands == 0) return;
    
    // Special case: single command
    if (pipeline->num_commands == 1) {
        // Check if it's a builtin command
        if (pipeline->commands[0].args[0] != NULL && 
            is_builtin_args(pipeline->commands[0].args)) {
            
            // Handle redirection for builtin commands in the parent process
            int saved_stdin = -1, saved_stdout = -1;
            int input_fd = -1, output_fd = -1;
            
            // Handle input redirection
            if (pipeline->commands[0].input_file) {
                input_fd = open(pipeline->commands[0].input_file, O_RDONLY);
                if (input_fd == -1) {
                    printf("No such file or directory\n");
                    return;
                }
                saved_stdin = dup(STDIN_FILENO);
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            
            // Handle output redirection
            if (pipeline->commands[0].output_file) {
                int flags = O_WRONLY | O_CREAT;
                if (pipeline->commands[0].append_output) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                output_fd = open(pipeline->commands[0].output_file, flags, 0644);
                if (output_fd == -1) {
                    printf("Unable to create file for writing\n");
                    if (saved_stdin != -1) {
                        dup2(saved_stdin, STDIN_FILENO);
                        close(saved_stdin);
                    }
                    return;
                }
                saved_stdout = dup(STDOUT_FILENO);
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            
            // Execute builtin command
            execute_builtin_args(pipeline->commands[0].args);
            
            // Restore stdin/stdout
            if (saved_stdin != -1) {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }
            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
            
        } else {
            // External command
            pid_t pid = fork();
            if (pid == 0) {
                // Child process - create new process group
                setpgid(0, 0);
                handle_redirection(&pipeline->commands[0]);
                execute_command(pipeline->commands[0].args);
                exit(1);
            } else if (pid > 0) {
                // Parent process
                setpgid(pid, pid);
                set_foreground_process(pid, pid);
                int status;
                waitpid(pid, &status, WUNTRACED);
                
                // Check if process was stopped
                if (WIFSTOPPED(status)) {
                    // Extract command name
                    char *command_name = pipeline->commands[0].args[0];
                    if (command_name == NULL) command_name = "unknown";
                    
                    // Reconstruct full command
                    char full_command[MAX_INPUT_SIZE] = "";
                    for (int i = 0; pipeline->commands[0].args[i] != NULL; i++) {
                        if (i > 0) strcat(full_command, " ");
                        strcat(full_command, pipeline->commands[0].args[i]);
                    }
                    
                    // Add to process list as stopped
                    add_process_to_list(pid, pid, command_name, full_command, "Stopped");
                    printf("[%d] Stopped %s\n", next_job_number - 1, command_name);
                }
                
                clear_foreground_process();
            } else {
                perror("fork");
            }
        }
        return;
    }
    
    // Multiple commands - create pipeline
    int pipes[MAX_PIPES-1][2];
    pid_t pids[MAX_PIPES];
    pgid_t pipeline_pgid = 0;
    
    // Create all pipes
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }
    
    // Fork and execute each command
    for (int i = 0; i < pipeline->num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Set process group (first process creates group, others join)
            if (i == 0) {
                setpgid(0, 0);
                pipeline_pgid = getpid();
            } else {
                setpgid(0, pipeline_pgid);
            }
            
            // Handle input redirection or pipe input
            if (i == 0) {
                // First command - check for input redirection
                if (pipeline->commands[i].input_file) {
                    int fd = open(pipeline->commands[i].input_file, O_RDONLY);
                    if (fd == -1) {
                        printf("No such file or directory\n");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
            } else {
                // Not first command - read from previous pipe
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Handle output redirection or pipe output
            if (i == pipeline->num_commands - 1) {
                // Last command - check for output redirection
                if (pipeline->commands[i].output_file) {
                    int flags = O_WRONLY | O_CREAT;
                    if (pipeline->commands[i].append_output) {
                        flags |= O_APPEND;
                    } else {
                        flags |= O_TRUNC;
                    }
                    int fd = open(pipeline->commands[i].output_file, flags, 0644);
                    if (fd == -1) {
                        printf("Unable to create file for writing\n");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            } else {
                // Not last command - write to next pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors in child
            for (int j = 0; j < pipeline->num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute command (builtin or external)
            if (is_builtin_args(pipeline->commands[i].args)) {
                execute_builtin_args(pipeline->commands[i].args);
                exit(0);
            } else {
                execute_command(pipeline->commands[i].args);
                exit(1);
            }
            
        } else if (pids[i] == -1) {
            perror("fork");
            return;
        } else {
            // Parent process - set process group
            if (i == 0) {
                pipeline_pgid = pids[i];
                setpgid(pids[i], pids[i]);
                set_foreground_process(pids[i], pipeline_pgid);
            } else {
                setpgid(pids[i], pipeline_pgid);
            }
        }
    }
    
    // Parent process - close all pipe file descriptors
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all child processes
    for (int i = 0; i < pipeline->num_commands; i++) {
        int status;
        waitpid(pids[i], &status, WUNTRACED);
        
        // Check if any process was stopped (only need to track the first one)
        if (i == 0 && WIFSTOPPED(status)) {
            char *command_name = pipeline->commands[0].args[0];
            if (command_name == NULL) command_name = "unknown";
            
            // Reconstruct full command for entire pipeline
            char full_command[MAX_INPUT_SIZE] = "";
            for (int cmd_idx = 0; cmd_idx < pipeline->num_commands; cmd_idx++) {
                if (cmd_idx > 0) strcat(full_command, " | ");
                for (int arg_idx = 0; pipeline->commands[cmd_idx].args[arg_idx] != NULL; arg_idx++) {
                    if (arg_idx > 0) strcat(full_command, " ");
                    strcat(full_command, pipeline->commands[cmd_idx].args[arg_idx]);
                }
            }
            
            add_process_to_list(pids[0], pipeline_pgid, command_name, full_command, "Stopped");
            printf("[%d] Stopped %s\n", next_job_number - 1, command_name);
        }
    }
    
    // Clear foreground process
    clear_foreground_process();
}

void handle_redirection(command_t *cmd) {
    // Handle input redirection
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1) {
            printf("No such file or directory\n");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // Handle output redirection
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->append_output) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        int fd = open(cmd->output_file, flags, 0644);
        if (fd == -1) {
            printf("Unable to create file for writing\n");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void execute_command(char **args) {
    if (args[0] == NULL) {
        return;
    }
    
    execvp(args[0], args);
    printf("Command not found!\n");
}

int is_builtin_args(char **args) {
    if (args[0] == NULL) return 0;
    
    return (strcmp(args[0], "hop") == 0) ||
           (strcmp(args[0], "reveal") == 0) ||
           (strcmp(args[0], "log") == 0) ||
           (strcmp(args[0], "activities") == 0) ||
           (strcmp(args[0], "ping") == 0) ||
           (strcmp(args[0], "fg") == 0) ||
           (strcmp(args[0], "bg") == 0);
}

int execute_builtin_args(char **args) {
    if (args[0] == NULL) return 0;
    
    // Count arguments
    int argc = 0;
    while (args[argc] != NULL) argc++;
    
    int result = 0;
    
    if (strcmp(args[0], "hop") == 0) {
        result = execute_hop(argc, args);
    } else if (strcmp(args[0], "reveal") == 0) {
        result = execute_reveal(argc, args);
    } else if (strcmp(args[0], "log") == 0) {
        result = execute_log(argc, args);
    } else if (strcmp(args[0], "activities") == 0) {
        result = execute_activities(argc, args);
    } else if (strcmp(args[0], "ping") == 0) {
        result = execute_ping(argc, args);
    } else if (strcmp(args[0], "fg") == 0) {
        result = execute_fg(argc, args);
    } else if (strcmp(args[0], "bg") == 0) {
        result = execute_bg(argc, args);
    }
    
    return result;
}

void free_pipeline(pipeline_t *pipeline) {
    for (int i = 0; i < pipeline->num_commands; i++) {
        command_t *cmd = &pipeline->commands[i];
        
        // Free arguments
        if (cmd->args) {
            for (int j = 0; cmd->args[j]; j++) {
                free(cmd->args[j]);
            }
            free(cmd->args);
        }
        
        // Free redirection files
        if (cmd->input_file) free(cmd->input_file);
        if (cmd->output_file) free(cmd->output_file);
    }
    
    if (pipeline->commands) {
        free(pipeline->commands);
    }
    
    pipeline->num_commands = 0;
    pipeline->commands = NULL;
}

// ############## LLM Generated Code Ends ################ 