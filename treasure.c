#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#define MAX_USERNAME 32
#define MAX_CLUE 128
#define RECORD_SIZE sizeof(struct Treasure)

struct Treasure {
    int id;
    char username[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
};

// Utility functions
void log_operation(const char *hunt_id, const char *operation) {
    char path[256];
    snprintf(path, sizeof(path), "%s/logged_hunt", hunt_id);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;

    dprintf(fd, "[%ld] %s\n", time(NULL), operation);
    close(fd);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name); // overwrite existing link
    symlink(path, symlink_name);
}

void ensure_hunt_dir(const char *hunt_id) {
    mkdir(hunt_id, 0755); // ignore if exists
}

const char* treasure_file_path(const char *hunt_id) {
    static char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);
    return path;
}

int next_id(const char *filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return 1;

    struct Treasure t;
    int max_id = 0;
    while (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        if (t.id > max_id) max_id = t.id;
    }

    close(fd);
    return max_id + 1;
}

// Commands
void add_treasure(const char *hunt_id) {
    ensure_hunt_dir(hunt_id);
    const char *filepath = treasure_file_path(hunt_id);

    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct Treasure t;
    t.id = next_id(filepath);

    printf("Enter username: ");
    fgets(t.username, MAX_USERNAME, stdin);
    t.username[strcspn(t.username, "\n")] = 0;

    printf("Enter latitude: "); scanf("%f", &t.latitude);
    printf("Enter longitude: "); scanf("%f", &t.longitude);
    getchar(); // consume newline

    printf("Enter clue: ");
    fgets(t.clue, MAX_CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;

    printf("Enter value: "); scanf("%d", &t.value);
    getchar(); // consume newline

    write(fd, &t, RECORD_SIZE);
    close(fd);

    log_operation(hunt_id, "Added treasure");
}

void list_treasures(const char *hunt_id) {
    const char *filepath = treasure_file_path(hunt_id);

    struct stat st;
    if (stat(filepath, &st) < 0) {
        perror("stat");
        return;
    }

    printf("Hunt: %s\nSize: %ld bytes\nLast Modified: %s",
           hunt_id, st.st_size, ctime(&st.st_mtime));

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct Treasure t;
    while (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        printf("ID: %d | User: %s | (%.2f, %.2f) | Clue: %s | Value: %d\n",
               t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }

    close(fd);
    log_operation(hunt_id, "Listed treasures");
}

void view_treasure(const char *hunt_id, int id) {
    const char *filepath = treasure_file_path(hunt_id);
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct Treasure t;
    while (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        if (t.id == id) {
            printf("Treasure ID: %d\nUser: %s\nGPS: %.2f, %.2f\nClue: %s\nValue: %d\n",
                   t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            log_operation(hunt_id, "Viewed treasure");
            close(fd);
            return;
        }
    }

    printf("Treasure ID %d not found.\n", id);
    close(fd);
}

void remove_treasure(const char *hunt_id, int id) {
    const char *filepath = treasure_file_path(hunt_id);
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    char tmpfile[256];
    snprintf(tmpfile, sizeof(tmpfile), "%s/tmp.dat", hunt_id);
    int fd_tmp = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0) {
        perror("temp file");
        close(fd);
        return;
    }

    struct Treasure t;
    int found = 0;
    while (read(fd, &t, RECORD_SIZE) == RECORD_SIZE) {
        if (t.id == id) {
            found = 1;
            continue;
        }
        write(fd_tmp, &t, RECORD_SIZE);
    }

    close(fd);
    close(fd_tmp);

    if (found) {
        rename(tmpfile, filepath);
        log_operation(hunt_id, "Removed treasure");
    } else {
        unlink(tmpfile);
        printf("Treasure ID %d not found.\n", id);
    }
}

void remove_hunt(const char *hunt_id) {
    DIR *dir = opendir(hunt_id);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char fullpath[256];

    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", hunt_id, entry->d_name);
            unlink(fullpath);
        }
    }
    closedir(dir);
    rmdir(hunt_id);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);

    printf("Hunt %s removed.\n", hunt_id);
    log_operation(hunt_id, "Removed hunt");
}

// Entry point
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s --<command> <hunt_id> [treasure_id]\n", argv[0]);
        return 1;
    }

    const char *cmd = argv[1] + 2;
    const char *hunt_id = argv[2];

    if (strcmp(cmd, "add") == 0) {
        add_treasure(hunt_id);
    } else if (strcmp(cmd, "list") == 0) {
        list_treasures(hunt_id);
    } else if (strcmp(cmd, "view") == 0 && argc == 4) {
        int id = atoi(argv[3]);
        view_treasure(hunt_id, id);
    } else if (strcmp(cmd, "remove_treasure") == 0 && argc == 4) {
        int id = atoi(argv[3]);
        remove_treasure(hunt_id, id);
    } else if (strcmp(cmd, "remove") == 0) {
        remove_hunt(hunt_id);
    } else {
        fprintf(stderr, "Unknown or malformed command.\n");
        return 1;
    }

    return 0;
}
