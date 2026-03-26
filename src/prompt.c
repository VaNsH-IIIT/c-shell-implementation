#include <shell.h>

void display_prompt(void) {
    char *username = get_username();
    char *hostname = get_hostname();
    char *current_path = get_current_directory();
    char *formatted_path = format_path_with_tilde(current_path);
    
    if (username != NULL && hostname != NULL && formatted_path != NULL) {
        printf("<%s@%s:[%s]> ", username, hostname, formatted_path);
        fflush(stdout);
    } else {
        printf("<shell>"); /* Fallback prompt */
        fflush(stdout);
    }
    
    /* Cleanup allocated memory */
    free(username);
    free(hostname);
    free(current_path);
    free(formatted_path);
}

char *get_username(void) {
    struct passwd *pwd;
    char *username = NULL;
    
    pwd = getpwuid(getuid());
    if (pwd != NULL && pwd->pw_name != NULL) {
        username = malloc(strlen(pwd->pw_name) + 1);
        if (username != NULL) {
            strcpy(username, pwd->pw_name);
        }
    }
    
    return username;
}

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

char *get_hostname(void) {
    char *hostname = malloc(256); /* Use a reasonable fixed size instead of HOST_NAME_MAX */
    if (hostname == NULL) {
        return NULL;
    }
    
    if (gethostname(hostname, 256) != 0) {
        free(hostname);
        return NULL;
    }
    
    /* Ensure null termination */
    hostname[255] = '\0';
    
    return hostname;
}

char *get_current_directory(void) {
    char *current_path = malloc(MAX_PATH_SIZE);
    if (current_path == NULL) {
        return NULL;
    }
    
    if (getcwd(current_path, MAX_PATH_SIZE) == NULL) {
        free(current_path);
        return NULL;
    }
    
    return current_path;
}

// ############## LLM Generated Code Begins ################

char *format_path_with_tilde(const char *current_path) {
    if (current_path == NULL || home_directory == NULL) {
        return NULL;
    }
    
    size_t home_len = strlen(home_directory);
    size_t current_len = strlen(current_path);
    
    /* Check if current path starts with home directory */
    if (current_len >= home_len && 
        strncmp(current_path, home_directory, home_len) == 0) {
        
        /* Check if it's exactly the home directory or a subdirectory */
        if (current_len == home_len) {
            /* Exactly home directory */
            char *result = malloc(2);
            if (result != NULL) {
                strcpy(result, "~");
            }
            return result;
        } else if (current_path[home_len] == '/') {
            /* It's a subdirectory of home */
            size_t remaining_len = current_len - home_len;
            char *result = malloc(remaining_len + 2); /* +2 for '~' and '\0' */
            if (result != NULL) {
                strcpy(result, "~");
                strcat(result, current_path + home_len);
            }
            return result;
        }
    }
    
    /* Path doesn't start with home directory, return as is */
    char *result = malloc(current_len + 1);
    if (result != NULL) {
        strcpy(result, current_path);
    }
    return result;
}

// ############## LLM Generated Code Ends ################