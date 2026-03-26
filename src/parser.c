// ############## LLM Generated Code Begins ##############

#include <shell.h>
#include <ctype.h>

void skip_whitespace(const char **ptr) {
    while (**ptr && (**ptr == ' ' || **ptr == '\t' || **ptr == '\n' || **ptr == '\r')) {
        (*ptr)++;
    }
}

int is_valid_name_char(char c) {
    /* Valid name characters are anything except |, &, >, <, ;, and whitespace */
    return c != '\0' && c != '|' && c != '&' && c != '>' && c != '<' && c != ';' && 
           c != ' ' && c != '\t' && c != '\n' && c != '\r';
}

int parse_name(const char **ptr, char *name_buf, size_t buf_size) {
    size_t i = 0;
    const char *start = *ptr;
    
    /* Parse valid name characters */
    while (is_valid_name_char(**ptr) && i < buf_size - 1) {
        if (name_buf != NULL) {
            name_buf[i] = **ptr;
        }
        (*ptr)++;
        i++;
    }
    
    if (name_buf != NULL) {
        name_buf[i] = '\0';
    }
    
    /* Return 1 if we parsed at least one character */
    return (*ptr > start) ? 1 : 0;
}

int parse_input_redirect(const char **ptr) {
    if (**ptr != '<') {
        return 0;
    }
    
    (*ptr)++; /* consume '<' */
    skip_whitespace(ptr);
    
    /* Must be followed by a name */
    return parse_name(ptr, NULL, 0);
}

int parse_output_redirect(const char **ptr) {
    if (**ptr != '>') {
        return 0;
    }
    
    (*ptr)++; /* consume first '>' */
    
    /* Check for '>>' */
    if (**ptr == '>') {
        (*ptr)++; /* consume second '>' */
    }
    
    skip_whitespace(ptr);
    
    /* Must be followed by a name */
    return parse_name(ptr, NULL, 0);
}

int parse_atomic(const char **ptr) {
    const char *checkpoint = *ptr;
    
    /* atomic -> name (name | input | output)* */
    
    /* Must start with a name */
    if (!parse_name(ptr, NULL, 0)) {
        *ptr = checkpoint;
        return 0;
    }
    
    /* Parse optional additional elements */
    while (**ptr) {
        skip_whitespace(ptr);
        
        /* Check if we've hit a terminator for atomic */
        if (**ptr == '|' || **ptr == '&' || **ptr == ';' || **ptr == '\0') {
            break;
        }
        
        /* Try to parse input redirect */
        if (**ptr == '<') {
            if (!parse_input_redirect(ptr)) {
                *ptr = checkpoint;
                return 0;
            }
            continue;
        }
        
        /* Try to parse output redirect */
        if (**ptr == '>') {
            if (!parse_output_redirect(ptr)) {
                *ptr = checkpoint;
                return 0;
            }
            continue;
        }
        
        /* Try to parse another name */
        if (is_valid_name_char(**ptr)) {
            if (!parse_name(ptr, NULL, 0)) {
                *ptr = checkpoint;
                return 0;
            }
            continue;
        }
        
        /* If we can't parse anything else, break */
        break;
    }
    
    return 1;
}

int parse_cmd_group(const char **ptr) {
    const char *checkpoint = *ptr;
    
    /* cmd_group -> atomic (\| atomic)* */
    
    /* Must start with an atomic */
    if (!parse_atomic(ptr)) {
        *ptr = checkpoint;
        return 0;
    }
    
    /* Parse optional pipe sequences */
    while (**ptr) {
        skip_whitespace(ptr);
        
        /* Check for pipe */
        if (**ptr == '|') {
            (*ptr)++; /* consume '|' */
            skip_whitespace(ptr);
            
            /* Must be followed by another atomic */
            if (!parse_atomic(ptr)) {
                *ptr = checkpoint;
                return 0;
            }
        } else {
            /* No more pipes, we're done with this cmd_group */
            break;
        }
    }
    
    return 1;
}

int parse_shell_command(const char *input) {
    const char *ptr = input;
    
    /* shell_cmd -> cmd_group ((& | ;) cmd_group)* &? */
    
    /* Skip initial whitespace */
    skip_whitespace(&ptr);
    
    /* Empty input is invalid */
    if (*ptr == '\0') {
        return 0;
    }
    
    /* Must start with a cmd_group */
    if (!parse_cmd_group(&ptr)) {
        return 0;
    }
    
    /* Parse optional separator sequences */
    while (*ptr) {
        skip_whitespace(&ptr);
        
        /* Check for separators (&, ;) */
        if (*ptr == '&' || *ptr == ';') {
            char separator = *ptr;
            ptr++; /* consume separator */
            skip_whitespace(&ptr);
            
            /* If we have a separator, check what follows */
            if (*ptr == '\0') {
                /* Separator at end is valid only for & */
                if (separator == '&') {
                    return 1; /* Valid background command */
                } else {
                    return 0; /* Invalid: ; at end */
                }
            } else {
                /* Must be followed by another cmd_group */
                if (!parse_cmd_group(&ptr)) {
                    return 0;
                }
            }
        } else if (*ptr == '\0') {
            /* End of input, we're done */
            break;
        } else {
            /* Unexpected character */
            return 0;
        }
    }
    
    /* Check for final optional & */
    skip_whitespace(&ptr);
    if (*ptr == '&') {
        ptr++;
        skip_whitespace(&ptr);
    }
    
    /* Should be at end of input */
    return (*ptr == '\0') ? 1 : 0;
}

// ############## LLM Generated Code Ends ################