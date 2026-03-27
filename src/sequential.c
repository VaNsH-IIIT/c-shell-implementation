#include <shell.h>

/*
 * Execute commands separated by ';' sequentially.
 * Waits for each to finish before continuing.
 */
void execute_sequential(char *input) {
    char *saveptr;
    char *cmd = strtok_r(input, ";", &saveptr);

    while (cmd != NULL) {
        // Trim leading spaces
        while (*cmd == ' ' || *cmd == '\t') {
            cmd++;
        }
        if (*cmd != '\0') {
            // Check if builtin and no redirection/pipe
            char *input_copy = strdup(cmd);
            char *first_token = strtok(input_copy, " \t");

            if (first_token != NULL && is_builtin_command(cmd) &&
                strchr(cmd, '|') == NULL && strchr(cmd, '<') == NULL &&
                strchr(cmd, '>') == NULL && strchr(cmd, '&') == NULL) {
                execute_builtin(cmd);
            } else {
                pipeline_t pipeline;
                parse_command_line(cmd, &pipeline);
                execute_pipeline(&pipeline);
                free_pipeline(&pipeline);
            }

            free(input_copy);
        }

        cmd = strtok_r(NULL, ";", &saveptr);
    }
}
