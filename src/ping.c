#include <shell.h>

int execute_ping(int argc, char **argv) {
    if (argc != 3) {
        printf("Invalid syntax!\n");
        return 1;
    }

    // Parse PID
    char *endptr;
    pid_t pid = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || pid <= 0) {
        printf("Invalid syntax!\n");
        return 1;
    }

    // Parse signal number
    int signal_number = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        printf("Invalid syntax!\n");
        return 1;
    }

    // Take signal number modulo 32
    int actual_signal = signal_number % 32;

    // Send signal to process
    if (kill(pid, actual_signal) == -1) {
        if (errno == ESRCH) {
            printf("No such process found\n");
        } else {
            printf("No such process found\n"); // Treat all errors as process not found
        }
        return 1;
    }

    printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    return 0;
}