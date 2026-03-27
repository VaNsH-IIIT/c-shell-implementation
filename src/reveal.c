#include <shell.h>

// ############## LLM Generated Code Begins ################

int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

int execute_reveal(int argc, char **argv) {
    int show_hidden = 0;
    int line_format = 0;
    char *target_dir = ".";
    int non_flag_args = 0;
    
    /* Parse flags and target directory */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* Process flag characters */
            for (int j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'a') {
                    show_hidden = 1;
                } else if (argv[i][j] == 'l') {
                    line_format = 1;
                }
                /* Ignore other characters in flags */
            }
        } else {
            /* Non-flag argument */
            non_flag_args++;
            if (non_flag_args > 1) {
                printf("reveal: Invalid Syntax!\n");
                return 1;
            }
            
            if (strcmp(argv[i], "~") == 0) {
                target_dir = home_directory;
            } else if (strcmp(argv[i], ".") == 0) {
                target_dir = ".";
            } else if (strcmp(argv[i], "..") == 0) {
                target_dir = "..";
            } else if (strcmp(argv[i], "-") == 0) {
                if (previous_directory == NULL) {
                    printf("No such directory!\n");
                    return 1;
                }
                target_dir = previous_directory;
            } else {
                target_dir = argv[i];
            }
        }
    }
    
    /* Open directory */
    DIR *dir = opendir(target_dir);
    if (dir == NULL) {
        printf("No such directory!\n");
        return 1;
    }
    
    /* Read directory entries */
    struct dirent *entry;
    char **entries = malloc(sizeof(char*) * 1000); /* Max 1000 entries */
    int entry_count = 0;
    
    if (entries == NULL) {
        closedir(dir);
        perror("malloc");
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        /* Skip hidden files unless -a flag is set */
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        
        entries[entry_count] = malloc(strlen(entry->d_name) + 1);
        if (entries[entry_count] == NULL) {
            /* Cleanup on malloc failure */
            for (int j = 0; j < entry_count; j++) {
                free(entries[j]);
            }
            free(entries);
            closedir(dir);
            perror("malloc");
            return 1;
        }
        strcpy(entries[entry_count], entry->d_name);
        entry_count++;
        
        if (entry_count >= 1000) {
            break; /* Prevent overflow */
        }
    }
    
    closedir(dir);
    
    /* Sort entries lexicographically */
    qsort(entries, entry_count, sizeof(char*), compare_strings);
    
    /* Print entries */
    if (line_format) {
        for (int i = 0; i < entry_count; i++) {
            printf("%s\n", entries[i]);
        }
    } else {
        for (int i = 0; i < entry_count; i++) {
            printf("%s", entries[i]);
            if (i < entry_count - 1) {
                printf(" ");
            }
        }
        if (entry_count > 0) {
            printf("\n");
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < entry_count; i++) {
        free(entries[i]);
    }
    free(entries);
    
    return 0;
}

// ############## LLM Generated Code Ends ################