#include <shell.h>
// ############## LLM Generated Code Begins ################
int compare_process_by_name(const void *a, const void *b) {
    const process_info_t *proc_a = (const process_info_t *)a;
    const process_info_t *proc_b = (const process_info_t *)b;
    return strcmp(proc_a->command_name, proc_b->command_name);
}

int execute_activities(int argc, char **argv) {
    if (argc != 1) {
        printf("Invalid syntax!\n");
        return 1;
    }
    
    // Update process statuses first
    update_process_list();

    // Get current processes
    process_info_t processes[MAX_PROCESSES];
    int count = get_active_processes(processes);

    if (count == 0) {
        return 0; // No processes to display
    }

    // Sort processes lexicographically by command name
    qsort(processes, count, sizeof(process_info_t), compare_process_by_name);

    // Print processes in the required format
    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - %s\n", 
               processes[i].pid, 
               processes[i].command_name, 
               processes[i].state);
    }

    return 0;
}
// ############## LLM Generated Code Ends ################ 