
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME 32

struct Treasure {
    int id;
    char username[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[128];
    int value;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_folder>", argv[0]);
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", argv[1]);

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        perror("Could not open treasure data file");
        return 1;
    }

    struct Treasure t;
    int score[1000] = {0};
    char usernames[1000][MAX_USERNAME];
    int user_index = 0;

    while (fread(&t, sizeof(struct Treasure), 1, fp)) {
        int found = 0;
        for (int i = 0; i < user_index; i++) {
            if (strcmp(usernames[i], t.username) == 0) {
                score[i] += t.value;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(usernames[user_index], t.username);
            score[user_index] = t.value;
            user_index++;
        }
    }

    fclose(fp);

    for (int i = 0; i < user_index; i++) {
        printf("%s: %d\n", usernames[i], score[i]);
    }

    return 0;
}
