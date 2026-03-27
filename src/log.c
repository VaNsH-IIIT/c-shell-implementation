#include <shell.h>
// ############## LLM Generated Code Begins ################
/* Command history storage */
static char *command_history[MAX_COMMAND_HISTORY];
static int history_count = 0;
static int history_start = 0;
static char *log_file_path = NULL;

void initialize_log(void) {
    /* Initialize history array */
    for (int i = 0; i < MAX_COMMAND_HISTORY; i++) {
        command_history[i] = NULL;
    }
    
    /* Create log file path */
    char *home = getenv("HOME");
    if (home == NULL) {
        struct passwd *pwd = getpwuid(getuid());
        if (pwd != NULL) {
            home = pwd->pw_dir;
        }
    }
    
    if (home != NULL) {
        log_file_path = malloc(strlen(home) + 20);
        sprintf(log_file_path, "%s/.shell_history", home);
        
        /* Load existing history from file */
        FILE *file = fopen(log_file_path, "r");
        if (file != NULL) {
            char line[MAX_INPUT_SIZE];
            while (fgets(line, sizeof(line), file) != NULL) {
                /* Remove newline */
                size_t len = strlen(line);
                if (len > 0 && line[len - 1] == '\n') {
                    line[len - 1] = '\0';
                }
                
                /* Add to history without saving to file again */
                if (history_count < MAX_COMMAND_HISTORY) {
                    command_history[history_count] = malloc(strlen(line) + 1);
                    strcpy(command_history[history_count], line);
                    history_count++;
                } else {
                    /* Overwrite oldest */
                    free(command_history[history_start]);
                    command_history[history_start] = malloc(strlen(line) + 1);
                    strcpy(command_history[history_start], line);
                    history_start = (history_start + 1) % MAX_COMMAND_HISTORY;
                }
            }
            fclose(file);
        }
    }
}

void cleanup_log(void) {
    /* Save history to file */
    if (log_file_path != NULL) {
        FILE *file = fopen(log_file_path, "w");
        if (file != NULL) {
            for (int i = 0; i < history_count; i++) {
                int index = (history_start + i) % MAX_COMMAND_HISTORY;
                if (command_history[index] != NULL) {
                    fprintf(file, "%s\n", command_history[index]);
                }
            }
            fclose(file);
        }
        free(log_file_path);
    }
    
    /* Free history memory */
    for (int i = 0; i < MAX_COMMAND_HISTORY; i++) {
        if (command_history[i] != NULL) {
            free(command_history[i]);
            command_history[i] = NULL;
        }
    }
}

int should_log_command(const char *command) {
    /* Skip empty commands */
    if (command == NULL || strlen(command) == 0) {
        return 0;
    }
    
    /* Skip commands that start with "log" */
    const char *trimmed = command;
    while (*trimmed == ' ' || *trimmed == '\t') {
        trimmed++;
    }
    
    if (strncmp(trimmed, "log", 3) == 0) {
        /* Check if it's exactly "log" or "log " */
        if (trimmed[3] == '\0' || trimmed[3] == ' ' || trimmed[3] == '\t') {
            return 0;
        }
    }
    
    /* Skip if identical to last command */
    if (history_count > 0) {
        int last_index = (history_start + history_count - 1) % MAX_COMMAND_HISTORY;
        if (command_history[last_index] != NULL && 
            strcmp(command, command_history[last_index]) == 0) {
            return 0;
        }
    }
    
    return 1;
}

void add_to_log(const char *command) {
    if (!should_log_command(command)) {
        return;
    }
    
    if (history_count < MAX_COMMAND_HISTORY) {
        /* Still have space */
        command_history[history_count] = malloc(strlen(command) + 1);
        strcpy(command_history[history_count], command);
        history_count++;
    } else {
        /* Overwrite oldest */
        free(command_history[history_start]);
        command_history[history_start] = malloc(strlen(command) + 1);
        strcpy(command_history[history_start], command);
        history_start = (history_start + 1) % MAX_COMMAND_HISTORY;
    }
}

void print_log(void) {
    for (int i = 0; i < history_count; i++) {
        int index = (history_start + i) % MAX_COMMAND_HISTORY;
        if (command_history[index] != NULL) {
            printf("%s\n", command_history[index]);
        }
    }
}

void purge_log(void) {
    /* Free all history entries */
    for (int i = 0; i < MAX_COMMAND_HISTORY; i++) {
        if (command_history[i] != NULL) {
            free(command_history[i]);
            command_history[i] = NULL;
        }
    }
    
    history_count = 0;
    history_start = 0;
    
    /* Clear the log file */
    if (log_file_path != NULL) {
        FILE *file = fopen(log_file_path, "w");
        if (file != NULL) {
            fclose(file);
        }
    }
}
// int execute_log_command(int cmd_index) {
//     if (cmd_index < 1 || cmd_index > history_count) {
//         printf("log: Invalid Syntax!\n");
//         return 1;
//     }
    
//     /* Convert to 0-based index (newest to oldest) */
//     int actual_index = (history_start + history_count - cmd_index) % MAX_COMMAND_HISTORY;
    
//     if (command_history[actual_index] == NULL) {
//         printf("log: Invalid Syntax!\n");
//         return 1;
//     }
    
//     /* Execute the command directly using pipeline parsing */
//     char *cmd_to_execute = malloc(strlen(command_history[actual_index]) + 1);
//     strcpy(cmd_to_execute, command_history[actual_index]);
    
//     /* Parse and execute the command using pipeline */
//     pipeline_t pipeline;
//     parse_command_line(cmd_to_execute, &pipeline);
//     execute_pipeline(&pipeline);
//     free_pipeline(&pipeline);
    
//     free(cmd_to_execute);
//     return 0;
// }
int execute_log_command(int cmd_index) {
    if (cmd_index < 1 || cmd_index > history_count) {
        printf("log: Invalid Syntax!\n");
        return 1;
    }
    
    /* Convert to 0-based index (newest to oldest) */
    int actual_index = (history_start + history_count - cmd_index) % MAX_COMMAND_HISTORY;
    
    if (command_history[actual_index] == NULL) {
        printf("log: Invalid Syntax!\n");
        return 1;
    }
    
    /* Execute the command */
    char *cmd_to_execute = malloc(strlen(command_history[actual_index]) + 1);
    strcpy(cmd_to_execute, command_history[actual_index]);

    // pipeline_t temp_pipeline;
    // if(temp_pipeline.num_commands > 0 && temp_pipeline.commands[0].args[0] != NULL){
    //     execvp(temp_pipeline.commands[0].args[0], temp_pipeline.commands[0].args);
    //     printf("log: Command execution failed!\n");
    //     exit(1);
    // }
    // else{
    //     printf("log: invalid command in history\n");
    //     exit(1);
    // }
    
    /* Parse and execute the command */
    if (parse_shell_command(cmd_to_execute)) {
        /* Check if it's a simple builtin command (no pipes/redirections) */
        char *input_copy = strdup(cmd_to_execute);
        char *first_token = strtok(input_copy, " \t");
        
        if (first_token != NULL && is_builtin_command(cmd_to_execute) && 
            strchr(cmd_to_execute, '|') == NULL && strchr(cmd_to_execute, '<') == NULL && 
            strchr(cmd_to_execute, '>') == NULL) {
            /* Simple builtin command - execute directly */
            execute_builtin(cmd_to_execute);
        } else {
            /* Complex command with pipes/redirections or external command */
            pipeline_t pipeline;
            parse_command_line(cmd_to_execute, &pipeline);
            execute_pipeline(&pipeline);
            free_pipeline(&pipeline);
        }
        
        free(input_copy);
    } else {
        printf("Invalid Syntax!\n");
    }
    
    free(cmd_to_execute);
    // free_pipeline(&temp_pipeline);
    return 0;
}

int execute_log(int argc, char **argv) {
    if (argc == 1) {
        /* Just "log" - print all history */
        print_log();
        return 0;
    }
    
    if (argc == 2) {
        if (strcmp(argv[1], "purge") == 0) {
            /* "log purge" - clear history */
            purge_log();
            return 0;
        } else {
            printf("log: Invalid Syntax!\n");
            return 1;
        }
    }
    
    if (argc == 3) {
        if (strcmp(argv[1], "execute") == 0) {
            /* "log execute <number>" - execute command from history */
            char *endptr;
            int cmd_index = strtol(argv[2], &endptr, 10);
            
            if (*endptr != '\0' || cmd_index <= 0) {
                printf("log: Invalid Syntax!\n");
                return 1;
            }
            
            return execute_log_command(cmd_index);
        } else {
            printf("log: Invalid Syntax!\n");
            return 1;
        }
    }
    
    printf("log: Invalid Syntax!\n");
    return 1;
}

// ############## LLM Generated Code Ends ################