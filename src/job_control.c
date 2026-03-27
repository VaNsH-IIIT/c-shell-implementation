#include <shell.h>
// ############## LLM Generated Code Begins ################
int execute_fg(int argc, char **argv) {
    int job_number = -1;
    
    if (argc > 2) {
        printf("Invalid syntax!\n");
        return 1;
    }
    
    if (argc == 2) {
        // Parse job number
        char *endptr;
        job_number = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || job_number <= 0) {
            printf("Invalid syntax!\n");
            return 1;
        }
    } else {
        // Use most recently created job
        job_number = get_most_recent_job_number();
        if (job_number == -1) {
            printf("No such job\n");
            return 1;
        }
    }
    
    // Find the job
    process_info_t *proc = find_process_by_job_number(job_number);
    if (proc == NULL) {
        printf("No such job\n");
        return 1;
    }
    
    // Print the entire command
    printf("%s\n", proc->full_command);
    
    // If the job is stopped, send SIGCONT to resume it
    if (strcmp(proc->state, "Stopped") == 0) {
        if (kill(proc->pid, SIGCONT) == -1) {
            printf("No such job\n");
            return 1;
        }
        strcpy(proc->state, "Running");
    }
    
    // Set as foreground process
    set_foreground_process(proc->pid, proc->pgid);
    
    // Wait for the job to complete or stop again
    int status;
    pid_t result = waitpid(proc->pid, &status, WUNTRACED);
    
    if (result > 0) {
        if (WIFSTOPPED(status)) {
            // Process was stopped (Ctrl-Z)
            strcpy(proc->state, "Stopped");
        } else {
            // Process terminated - remove from list
            remove_process_from_list(proc->pid);
        }
    }
    
    // Clear foreground process
    clear_foreground_process();
    
    return 0;
}

int execute_bg(int argc, char **argv) {
    int job_number = -1;
    
    if (argc > 2) {
        printf("Invalid syntax!\n");
        return 1;
    }
    
    if (argc == 2) {
        // Parse job number
        char *endptr;
        job_number = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || job_number <= 0) {
            printf("Invalid syntax!\n");
            return 1;
        }
    } else {
        // Use most recently created job
        job_number = get_most_recent_job_number();
        if (job_number == -1) {
            printf("No such job\n");
            return 1;
        }
    }
    
    // Find the job
    process_info_t *proc = find_process_by_job_number(job_number);
    if (proc == NULL) {
        printf("No such job\n");
        return 1;
    }
    
    // Check if job is already running
    if (strcmp(proc->state, "Running") == 0) {
        printf("Job already running\n");
        return 1;
    }
    
    // Only stopped jobs can be resumed
    if (strcmp(proc->state, "Stopped") != 0) {
        printf("No such job\n");
        return 1;
    }
    
    // Send SIGCONT to resume the job
    if (kill(proc->pid, SIGCONT) == -1) {
        printf("No such job\n");
        return 1;
    }
    
    // Update state and print status
    strcpy(proc->state, "Running");
    printf("[%d] %s &\n", proc->job_number, proc->command_name);
    
    return 0;
}

// ############## LLM Generated Code Ends ################