#ifndef SHELL_H
#define SHELL_H
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

/* Type definitions */
typedef pid_t pgid_t;

/* Constants */
#define MAX_INPUT_SIZE 1024
#define MAX_PATH_SIZE 4096
#define MAX_COMMAND_HISTORY 15
#define MAX_ARGS 100
#define MAX_PIPES 50
#define MAX_PROCESSES 100

/* Structures for command parsing and execution */
typedef struct {
    pid_t pid;
    pgid_t pgid;
    int job_number;
    char command_name[256];
    char full_command[MAX_INPUT_SIZE];
    char state[16]; // "Running" or "Stopped"
} process_info_t;

typedef struct {
    char **args;
    char *input_file;
    char *output_file;
    int append_output;  // 1 for >>, 0 for >
} command_t;

typedef struct {
    command_t *commands;
    int num_commands;
} pipeline_t;

/* Global variables */
extern char *home_directory;
extern char *previous_directory;

/* Process management */
extern process_info_t process_list[MAX_PROCESSES];
extern int process_count;
extern int next_job_number;

/* Function declarations */

/* prompt.c */
void display_prompt(void);
char *get_username(void);
char *get_hostname(void);
char *get_current_directory(void);
char *format_path_with_tilde(const char *current_path);

/* parser.c */
int parse_shell_command(const char *input);
void skip_whitespace(const char **ptr);
int is_valid_name_char(char c);
int parse_name(const char **ptr, char *name_buf, size_t buf_size);
int parse_input_redirect(const char **ptr);
int parse_output_redirect(const char **ptr);
int parse_atomic(const char **ptr);
int parse_cmd_group(const char **ptr);

/* builtin.c */
int is_builtin_command(const char *command);
int execute_builtin(const char *input);
void split_command_args(const char *input, int *argc, char ***argv);
void free_command_args(int argc, char **argv);

/* hop.c */
int execute_hop(int argc, char **argv);

/* reveal.c */
int execute_reveal(int argc, char **argv);
int compare_strings(const void *a, const void *b);

/* log.c (builtin part) */
int execute_log(int argc, char **argv);

/* activities.c - E.1 */
int execute_activities(int argc, char **argv);
int compare_process_by_name(const void *a, const void *b);

/* ping.c - E.2 */
int execute_ping(int argc, char **argv);

/* signals.c - E.3 */
void setup_signal_handlers(void);
void set_foreground_process(pid_t pid, pgid_t pgid);
void clear_foreground_process(void);
int check_eof_and_exit(void);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

/* jobcontrol.c - E.4 */
int execute_fg(int argc, char **argv);
int execute_bg(int argc, char **argv);

/* log.c */
void initialize_log(void);
void cleanup_log(void);
void add_to_log(const char *command);
void print_log(void);
void purge_log(void);
int execute_log_command(int index);
int should_log_command(const char *command);

/* executor.c - New file for command execution */
void parse_command_line(char *line, pipeline_t *pipeline);
void execute_pipeline(pipeline_t *pipeline);
void free_pipeline(pipeline_t *pipeline);
char **tokenize_command(char *cmd_str, int *argc);
void handle_redirection(command_t *cmd);
void execute_command(char **args);
int is_builtin_args(char **args);
int execute_builtin_args(char **args);

/* Process management functions */

/* sequential.c */
void execute_sequential(char *input);

/* background.c */
void execute_background(char *cmd);
void check_background_processes(void);

/* main.c */
int main(void);
void initialize_shell(void);
void cleanup_shell(void);

/* process_mgmt.c */
void add_process_to_list(pid_t pid, pgid_t pgid, const char *command_name, const char *full_command, const char *state);
process_info_t* find_process_by_pid(pid_t pid);
process_info_t* find_process_by_job_number(int job_number);
int get_most_recent_job_number(void);
void remove_process_from_list(pid_t pid);
void update_process_list(void);
int get_active_processes(process_info_t *processes);
void kill_all_child_processes(void);

#endif /* SHELL_H */