#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define REQUEST_FILE "monitor_request.txt"
pid_t monitor_pid = -1;

// Signal handler for SIGCHLD (monitor ended)
void sigchld_handler(int signo) {
    int status;
    waitpid(monitor_pid, &status, WNOHANG);
    monitor_pid = -1;
    printf("[INFO] Monitor process terminated.\n");
}

void start_monitor() {
    if (monitor_pid > 0) {
        printf("[ERROR] Monitor already running.\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == 0) {
        // Child process -> start monitor
        execl("./monitor", "./monitor", NULL);
        perror("execl failed");
        exit(1);
    } else if (monitor_pid > 0) {
        printf("[INFO] Monitor started. PID: %d\n", monitor_pid);
    } else {
        perror("fork failed");
    }
}

void send_request(const char *command) {
    if (monitor_pid < 0) {
        printf("[ERROR] Monitor not running.\n");
        return;
    }

    // Write command to request file
    FILE *f = fopen(REQUEST_FILE, "w");
    if (!f) {
        perror("fopen");
        return;
    }
    fprintf(f, "%s\n", command);
    fclose(f);

    // Send SIGUSR1 to monitor
    kill(monitor_pid, SIGUSR1);
}

void stop_monitor() {
    if (monitor_pid < 0) {
        printf("[ERROR] Monitor not running.\n");
        return;
    }

    // Write stop command
    send_request("stop_monitor");

    // Wait for SIGCHLD to confirm termination
    printf("[INFO] Waiting for monitor to terminate...\n");
    sleep(2); // Slight delay to allow monitor to clean up
}

int main() {
    // Set up SIGCHLD handler
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    char command[256];

    while (1) {
        printf("\n> Enter command: ");
        if (!fgets(command, sizeof(command), stdin)) break;
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(command, "list_hunts") == 0 ||
                   strncmp(command, "list_treasures", 14) == 0 ||
                   strncmp(command, "view_treasure", 13) == 0) {
            send_request(command);
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_pid > 0) {
                printf("[ERROR] Monitor still running. Stop it first.\n");
            } else {
                break;
            }
        } else {
            printf("[ERROR] Unknown command.\n");
        }
    }

    return 0;
}
