#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

void create_hunt(const char *name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", name);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "touch %s/logged_hunt", name);
    system(cmd);
    printf("Hunt %s created.\n", name);
}

void run_monitor(const char *name) {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", pipefd[1]);
        setenv("PIPE_WRITE_FD", fd_str, 1);
        execl("./monitor", "monitor", name, NULL);
        perror("execl");
        exit(1);
    } else {
        close(pipefd[1]);
        char buffer[256];
        while (read(pipefd[0], buffer, sizeof(buffer)) > 0) {
            printf("%s", buffer);
        }
        close(pipefd[0]);
        wait(NULL);
    }
}

void calculate_score(const char *name) {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        execl("./calculate_score", "calculate_score", name, NULL);
        perror("execl");
        exit(1);
    } else {
        close(pipefd[1]);
        char buffer[256];
        int n;
        while ((n = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);
        }
        close(pipefd[0]);
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <command> <hunt_name>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "createhunt") == 0) {
        create_hunt(argv[2]);
    } else if (strcmp(argv[1], "monitor") == 0) {
        run_monitor(argv[2]);
    } else if (strcmp(argv[1], "calculate_score") == 0) {
        calculate_score(argv[2]);
    } else {
        printf("Unknown command.\n");
    }

    return 0;
}
