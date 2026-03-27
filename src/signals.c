#include <shell.h>

static pid_t current_foreground_pid = -1;
static pgid_t current_foreground_pgid = -1;
// ############## LLM Generated Code Begins ################
void sigint_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    if (current_foreground_pgid > 0) {
        // Send SIGINT to the foreground process group
        kill(-current_foreground_pgid, SIGINT);
    }
    // Don't terminate the shell itself
}


void sigtstp_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    if (current_foreground_pgid > 0 && current_foreground_pid > 0) {
        // Send SIGTSTP to the foreground process group
        kill(-current_foreground_pgid, SIGTSTP);
        
        // Find the process in our list and move it to background with "Stopped" status
        process_info_t *proc = find_process_by_pid(current_foreground_pid);
        if (proc != NULL) {
            strcpy(proc->state, "Stopped");
            printf("\n[%d] Stopped %s\n", proc->job_number, proc->command_name);
        }
        
        // Clear current foreground process
        current_foreground_pid = -1;
        current_foreground_pgid = -1;
    }
    // Don't stop the shell itself
}

void setup_signal_handlers(void) {
    struct sigaction sa_int, sa_tstp;
    
    // Setup SIGINT handler
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);
    
    // Setup SIGTSTP handler
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}
// ############## LLM Generated Code Ends ################

void set_foreground_process(pid_t pid, pgid_t pgid) {
    current_foreground_pid = pid;
    current_foreground_pgid = pgid;
}

void clear_foreground_process(void) {
    current_foreground_pid = -1;
    current_foreground_pgid = -1;
}

int check_eof_and_exit(void) {
    // Send SIGKILL to all child processes
    kill_all_child_processes();
    
    printf("logout\n");
    exit(0);
}