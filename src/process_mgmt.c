#include <shell.h>

// ############## LLM Generated Code Begins ################

void add_process_to_list(pid_t pid, pgid_t pgid, const char *command_name, const char *full_command, const char *state) {
    if (process_count >= MAX_PROCESSES) {
        return; // List is full
    }
    
    process_list[process_count].pid = pid;
    process_list[process_count].pgid = pgid;
    process_list[process_count].job_number = next_job_number++;
    
    strncpy(process_list[process_count].command_name, command_name, sizeof(process_list[process_count].command_name) - 1);
    process_list[process_count].command_name[sizeof(process_list[process_count].command_name) - 1] = '\0';
    
    strncpy(process_list[process_count].full_command, full_command, sizeof(process_list[process_count].full_command) - 1);
    process_list[process_count].full_command[sizeof(process_list[process_count].full_command) - 1] = '\0';
    
    strncpy(process_list[process_count].state, state, sizeof(process_list[process_count].state) - 1);
    process_list[process_count].state[sizeof(process_list[process_count].state) - 1] = '\0';
    
    process_count++;
}

process_info_t* find_process_by_pid(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            return &process_list[i];
        }
    }
    return NULL;
}

process_info_t* find_process_by_job_number(int job_number) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].job_number == job_number) {
            return &process_list[i];
        }
    }
    return NULL;
}

int get_most_recent_job_number(void) {
    if (process_count == 0) {
        return -1;
    }
    
    int most_recent_job = 0;
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].job_number > most_recent_job) {
            most_recent_job = process_list[i].job_number;
        }
    }
    return most_recent_job;
}

void remove_process_from_list(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            // Shift all subsequent processes down
            for (int j = i; j < process_count - 1; j++) {
                process_list[j] = process_list[j + 1];
            }
            process_count--;
            break;
        }
    }
}

void update_process_list(void) {
    int status;
    pid_t pid;
    
    // Check for any process state changes
    for (int i = 0; i < process_count; i++) {
        pid = waitpid(process_list[i].pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        
        if (pid == process_list[i].pid) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // Process terminated - will be removed by caller if needed
                continue;
            } else if (WIFSTOPPED(status)) {
                strcpy(process_list[i].state, "Stopped");
            } else if (WIFCONTINUED(status)) {
                strcpy(process_list[i].state, "Running");
            }
        } else if (pid == 0) {
            // No change in status
            continue;
        } else if (pid == -1) {
            // Error or process doesn't exist - mark for removal
            if (errno == ECHILD) {
                // Process no longer exists
                remove_process_from_list(process_list[i].pid);
                i--; // Adjust index since we removed an element
            }
        }
    }
}

int get_active_processes(process_info_t *processes) {
    int count = 0;
    
    for (int i = 0; i < process_count; i++) {
        // Check if process is still alive
        if (kill(process_list[i].pid, 0) == 0) {
            processes[count] = process_list[i];
            count++;
        } else {
            // Process no longer exists, remove it
            remove_process_from_list(process_list[i].pid);
            i--; // Adjust index
        }
    }
    
    return count;
}

void kill_all_child_processes(void) {
    for (int i = 0; i < process_count; i++) {
        kill(process_list[i].pid, SIGKILL);
    }
    
    // Wait a bit for processes to die
    sleep(1); // 1 second - sufficient for SIGKILL to take effect
    
    // Clear the process list
    process_count = 0;
}

// ############## LLM Generated Code Ends ################