#include <shell.h>

int is_builtin_command(const char *command) {
    return (strncmp(command, "hop", 3) == 0) ||
           (strncmp(command, "reveal", 6) == 0) ||
           (strncmp(command, "log", 3) == 0);
}

void split_command_args(const char *input, int *argc, char ***argv) {
    char *input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    
    *argc = 0;
    *argv = malloc(sizeof(char*) * 64); /* Max 64 arguments */
    
    char *token = strtok(input_copy, " \t\n\r");
    while (token != NULL && *argc < 63) {
        (*argv)[*argc] = malloc(strlen(token) + 1);
        strcpy((*argv)[*argc], token);
        (*argc)++;
        token = strtok(NULL, " \t\n\r");
    }
    (*argv)[*argc] = NULL;
    
    free(input_copy);
}

void free_command_args(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}

int execute_builtin(const char *input) {
    int argc;
    char **argv;
    
    split_command_args(input, &argc, &argv);
    
    if (argc == 0) {
        free_command_args(argc, argv);
        return 0;
    }
    
    int result = 0;
    
    if (strcmp(argv[0], "hop") == 0) {
        result = execute_hop(argc, argv);
    } else if (strcmp(argv[0], "reveal") == 0) {
        result = execute_reveal(argc, argv);
    } else if (strcmp(argv[0], "log") == 0) {
        result = execute_log(argc, argv);
    }
    
    free_command_args(argc, argv);
    return result;
}