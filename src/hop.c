#include <shell.h>

int execute_hop(int argc, char **argv) {
    char current_dir[MAX_PATH_SIZE];
    
    /* Get current directory before any changes */
    // ############## LLM Generated Code Begins ################
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return 1;
    }
    
    
    /* If no arguments, go to home */
    if (argc == 1) {
        if (chdir(home_directory) != 0) {
            printf("No such directory!\n");
            return 1;
        }
        /* Update previous directory */
        if (previous_directory != NULL) {
            free(previous_directory);
        }
        previous_directory = malloc(strlen(current_dir) + 1);
        if (previous_directory != NULL) {
            strcpy(previous_directory, current_dir);
        }
        return 0;
        // ############## LLM Generated Code Ends ################ 
    }
    
    /* Process each argument sequentially */
    for (int i = 1; i < argc; i++) {
        char *target_dir = NULL;
        
        if (strcmp(argv[i], "~") == 0) {
            target_dir = home_directory;
        } else if (strcmp(argv[i], ".") == 0) {
            /* Do nothing - stay in current directory */
            continue;
        } else if (strcmp(argv[i], "..") == 0) {
            target_dir = "..";
        } else if (strcmp(argv[i], "-") == 0) {
            if (previous_directory == NULL) {
                /* No previous directory, do nothing */
                continue;
            } else {
                target_dir = previous_directory;
            }
        } else {
            /* Regular path */
            target_dir = argv[i];
        }
        
        /* Try to change to target directory */
        if (target_dir != NULL) {
            /* Get current directory before change for previous_directory update */
            char before_change[MAX_PATH_SIZE];
            if (getcwd(before_change, sizeof(before_change)) == NULL) {
                perror("getcwd");
                return 1;
            }
            
            if (chdir(target_dir) != 0) {
                printf("No such directory!\n");
                return 1;
            }
            
            /* Update previous directory */
            if (previous_directory != NULL) {
                free(previous_directory);
            }
            previous_directory = malloc(strlen(before_change) + 1);
            if (previous_directory != NULL) {
                strcpy(previous_directory, before_change);
            }
        }
    }
    
    return 0;
}