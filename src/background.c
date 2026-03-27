#include <shell.h>
// ############## LLM Generated Code Begins ################ 
/*
 * Launch a command in background (&).
 * Does not wait for it, prints job info, and returns.
 */
void execute_background(char *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child: detach from terminal input and create new process group
        int fd = open("/dev/null", O_RDONLY);
        if (fd >= 0) {
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        // Create new process group
        setpgid(0, 0);

        // Execute
        char *input_copy = strdup(cmd);
        char *first_token = strtok(input_copy, " \t");

        if (first_token != NULL && is_builtin_command(cmd) &&
            strchr(cmd, '|') == NULL && strchr(cmd, '<') == NULL &&
            strchr(cmd, '>') == NULL) {
            execute_builtin(cmd);
        } else {
            pipeline_t pipeline;
            parse_command_line(cmd, &pipeline);
            execute_pipeline(&pipeline);
            free_pipeline(&pipeline);
        }

        free(input_copy);
        exit(EXIT_SUCCESS);
    } else {
        // Parent: set process group and add to process list
        setpgid(pid, pid);
        
        // Extract command name (first word)
        char *cmd_copy = strdup(cmd);
        char *command_name = strtok(cmd_copy, " \t");
        if (command_name == NULL) {
            command_name = cmd;
        }
        
        // Add to process list
        add_process_to_list(pid, pid, command_name, cmd, "Running");
        
        printf("[%d] %d\n", next_job_number - 1, pid);
        free(cmd_copy);
    }
}

/*
 * Check if any background process has finished.
 * Prints appropriate status message.
 */
void check_background_processes(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        process_info_t *proc = find_process_by_pid(pid);
        if (proc != NULL) {
            if (WIFEXITED(status)) {
                printf("Process with pid %d exited normally\n", pid);
            } else {
                printf("Process with pid %d exited abnormally\n", pid);
            }
            remove_process_from_list(pid);
        }
    }
}
// ############## LLM Generated Code Ends ################ 