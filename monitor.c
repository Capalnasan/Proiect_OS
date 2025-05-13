#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

void monitor_hunt(const char *hunt_name, int write_fd) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_name);

    FILE *log_file = fopen(log_path, "r");
    if (!log_file) {
        dprintf(write_fd, "Could not open log file for %s\n", hunt_name);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), log_file)) {
        dprintf(write_fd, "%s", line);
    }

    fclose(log_file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_name>\n", argv[0]);
        return 1;
    }

    char *fd_str = getenv("PIPE_WRITE_FD");
    if (!fd_str) {
        fprintf(stderr, "PIPE_WRITE_FD not set\n");
        return 1;
    }

    int write_fd = atoi(fd_str);
    monitor_hunt(argv[1], write_fd);

    return 0;
}

