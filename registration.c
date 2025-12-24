#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "user.h"

void registration(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database) {
    int action = -1;
    char name[51];
    char password[31];
    int argc = sscanf(buffer, ":%d:%50[^:]:%30[^:]:", &action, name, password);
    printf("Receive from %d: %s", fd, buffer);
    if (argc < 3) {
        printf("Bad format from %d\n", fd);
        char message[1024];
        int message_lenght = sprintf(message, "Bad format.\n");
        if (send(fd, message, message_lenght, 0) == -1) {
            perror("send");
        }
    } else {
        if (action == 0) {
            printf("Socket %d want to create account with name, password: %s, %s\n", fd, name, password);
            memcpy(users[fd].name, name, sizeof(password));
            memcpy(users[fd].password, password, sizeof(password));
            if (user_save(database, &users[fd]) == 0) {
                users[fd].logged = 1;
                printf("Socket %d succesfuly create account with name, password: %s, %s\n", fd, users[fd].name, password);
                char message[1024];
                int message_lenght = sprintf(message, "Succesful!\nWelcome %s\n", name);
                if (send(fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            } else {
                printf("Socket %d unsuccesfuly create account with name, password: %s, %s\n", fd, users[fd].name, password);
                char message[1024];
                int message_lenght = sprintf(message, "This name was used, try again.\n");
                if (send(fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            }
        }
        if (action == 1) {
            printf("Socket %d want to log into account with name, password: %s, %s\n", fd, name, password);
            memcpy(users[fd].name, name, sizeof(password));
            memcpy(users[fd].password, password, sizeof(password));
            int errcode = user_get(database, &users[fd]);
            if (errcode == 0) {
                users[fd].logged = 1;
                printf("Socket %d succesfuly logged into account with name, password: %s, %s\n", fd, users[fd].name, password);
                char message[1024];
                int message_lenght = sprintf(message, "Succesful!\nWelcome %s\n", name);
                if (send(fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            }
            if (errcode == -2) {
                printf("Socket %d unsuccesfuly logged into account with name, password: %s, %s\n", fd, users[fd].name, password);
                char message[1024];
                int message_lenght = sprintf(message, "Unuccesful, wrong password.\n");
                if (send(fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            }
            if (errcode == -3) {
                printf("Socket %d unsuccesfuly logged into account with name, password: %s, %s\n", fd, users[fd].name, password);
                char message[1024];
                int message_lenght = sprintf(message, "Unuccesful, account don't exist.\n");
                if (send(fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            }
        }
        if (action != 0 && action != 1) {
            printf("Socket %d bad action.\n", fd);
        }
    }
}