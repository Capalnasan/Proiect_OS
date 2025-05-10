#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define REQUEST_FILE "monitor_request.txt"

volatile sig_atomic_t got_request = 0;

void sigusr1_handler(int signo) {
    got_request = 1;
}

void process_command() {
    FILE *f = fopen(REQUEST_FILE, "r");
    if (!f) {
        perror("[Monitor] Failed to open request file");
        return;
    }

    char command[256];
    if (!fgets(command, sizeof(command), f)) {
        fclose(f);
        return;
    }
    command[strcspn(command, "\n")] = 0;
    fclose(f);

    printf("[Monitor] Received command: %s\n", command);

    if (strcmp(command, "list_hunts") == 0) {
        system("ls -d */ | cut -f1 -d'/'"); // List directories (hunts)
    } else if (strncmp(command, "list_treasures", 14) == 0) {
        char hunt[128];
        sscanf(command, "list_treasures %s", hunt);
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./treasure_manager --list %s", hunt);
        system(cmd);
    } else if (strncmp(command, "view_treasure", 13) == 0) {
        char hunt[128];
        int id;
        sscanf(command, "view_treasure %s %d", hunt, &id);
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./treasure_manager --view %s %d", hunt, id);
        system(cmd);
    } else if (strcmp(command, "stop_monitor") == 0) {
        printf("[Monitor] Stopping monitor as requested.\n");
        sleep(1); // Delay to simulate usleep()
        exit(0);
    } else {
        printf("[Monitor] Unknown command.\n");
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    printf("[Monitor] Ready. PID: %d\n", getpid());

    while (1) {
        pause(); // Wait for signal
        if (got_request) {
            got_request = 0;
            process_command();
        }
    }

    return 0;
}

/*

gcc treasure_manager.c -o treasure_manager
gcc treasure_hub.c -o treasure_hub
gcc monitor.c -o monitor


*/