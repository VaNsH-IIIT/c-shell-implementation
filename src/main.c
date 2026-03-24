#include <shell.h>
// ############## LLM Generated Code Begins ################

/* Global variable to store home directory */
char *home_directory = NULL;
char *previous_directory = NULL;

/* Process management globals */
process_info_t process_list[MAX_PROCESSES];
int process_count = 0;
int next_job_number = 1;

void initialize_shell(void) {
    char cwd[MAX_PATH_SIZE];

    /* Get current working directory as home directory */
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        home_directory = malloc(strlen(cwd) + 1);
        if (home_directory == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(home_directory, cwd);
    } else {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    /* Initialize command history */
    initialize_log();
    
    /* Initialize process management */
    process_count = 0;
    next_job_number = 1;
    
    /* Setup signal handlers */
    setup_signal_handlers();
}

void cleanup_shell(void) {
    if (home_directory != NULL) {
        free(home_directory);
        home_directory = NULL;
    }

    if (previous_directory != NULL) {
        free(previous_directory);
        previous_directory = NULL;
    }

    /* Cleanup command history */
    cleanup_log();
}

int main(void) {
    char input[MAX_INPUT_SIZE];
    pipeline_t pipeline;

    /* Initialize shell */
    initialize_shell();

    /* Main shell loop */
    while (1) {
        /* First, check for completed background processes */
        check_background_processes();

        /* Display prompt */
        display_prompt();

        /* Read input */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            /* EOF reached (Ctrl-D) */
            check_eof_and_exit();
        }

        /* Remove newline from input if present */
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        /* Skip empty input */
        if (strlen(input) == 0) {
            continue;
        }

        /* Parse the input according to the grammar */
        if (parse_shell_command(input)) {
            /* Valid syntax - add to log */
            add_to_log(input);

            /* --- Sequential execution --- */
            if (strchr(input, ';') != NULL) {
                execute_sequential(input);
                continue;
            }

            /* --- Background execution --- */
            if (input[strlen(input) - 1] == '&') {
                input[strlen(input) - 1] = '\0';   // trim '&'
                while (strlen(input) > 0 &&
                       (input[strlen(input) - 1] == ' ' || input[strlen(input) - 1] == '\t')) {
                    input[strlen(input) - 1] = '\0'; // trim trailing spaces
                }
                if (strlen(input) > 0) {
                    execute_background(input);
                }
                continue;
            }

            /* --- Normal execution (builtin or pipeline) --- */
            char *input_copy = strdup(input);
            char *first_token = strtok(input_copy, " \t");

            if (first_token != NULL && is_builtin_command(input) &&
                strchr(input, '|') == NULL && strchr(input, '<') == NULL &&
                strchr(input, '>') == NULL) {
                /* Simple builtin command - execute directly */
                execute_builtin(input);
            } else {
                /* Complex command with pipes/redirections or external command */
                parse_command_line(input, &pipeline);
                execute_pipeline(&pipeline);
                free_pipeline(&pipeline);
            }

            free(input_copy);
        } else {
            /* Invalid syntax */
            printf("Invalid Syntax!\n");
        }
    }

    /* Cleanup and exit */
    cleanup_shell();
    return 0;
}

// ############## LLM Generated Code Ends ################