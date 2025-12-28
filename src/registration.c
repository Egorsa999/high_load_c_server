#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "config.h"
#include "network.h"

void registration(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *poll_struct) {
    int action = -1;
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    memset(name, 0, sizeof(name));
    memset(password, 0, sizeof(password));

    //parse command as :type:name:password:
    int argc = sscanf(buffer, ":%d:%" USERNAME_SIZES "[^:]:%" PASSWORD_SIZES "[^:]:", &action, name, password);
    printf("Receive from %d: %s\n", fd, buffer);
    if (argc < 3) {
        printf("Bad format from %d\n", fd);
        char message[SEND_SIZE];
        unsigned int message_lenght = snprintf(message, sizeof(message), "Bad format.\n");
        if (message_lenght >= sizeof(message)) {
            message_lenght = sizeof(message) - 1;
        }
        if (send_message(fd, &users[fd], poll_struct, message, message_lenght) == -1) {
            perror("send");
        }
    } else {
        if (action == 0) {
            printf("Socket %d want to create account with name, password: %s, %s\n", fd, name, password);
            strncpy(users[fd].name, name, USERNAME_SIZE - 1);
            strncpy(users[fd].password, password, PASSWORD_SIZE - 1);
            users[fd].name[USERNAME_SIZE - 1] = '\0';
            users[fd].password[PASSWORD_SIZE - 1] = '\0';

            char message[SEND_SIZE];
            unsigned int message_lenght;
            if (user_save(database, &users[fd]) == 0) {
                users[fd].logged = 1;
                printf("Socket %d succesfuly create account with name, password: %s, %s\n", fd, users[fd].name, password);
                message_lenght = snprintf(message, sizeof(message), "Succesful!\nWelcome %s\n", name);
            } else {
                printf("Socket %d unsuccesfuly create account with name, password: %s, %s\n", fd, users[fd].name, password);
                message_lenght = snprintf(message, sizeof(message),"This name was used, try again.\n");
            }

            if (message_lenght >= sizeof(message)) {
                message_lenght = sizeof(message) - 1;
            }
            if (send_message(fd, &users[fd], poll_struct, message, message_lenght) == -1) {
                perror("send");
            }
        }
        if (action == 1) {
            printf("Socket %d want to log into account with name, password: %s, %s\n", fd, name, password);
            strncpy(users[fd].name, name, USERNAME_SIZE - 1);
            strncpy(users[fd].password, password, PASSWORD_SIZE - 1);
            users[fd].name[USERNAME_SIZE - 1] = '\0';
            users[fd].password[PASSWORD_SIZE - 1] = '\0';

            int errcode = user_get(database, &users[fd]);
            char message[SEND_SIZE];
            unsigned int message_lenght;
            if (errcode == 0) {
                users[fd].logged = 1;
                printf("Socket %d successfully logged into account with name, password: %s, %s\n", fd, users[fd].name, password);
                message_lenght = snprintf(message, sizeof(message), "Successful!\nWelcome %s\n", name);
            }
            if (errcode == -2) {
                printf("Socket %d unsuccessfully logged into account with name, password: %s, %s\n", fd, users[fd].name, password);
                message_lenght = snprintf(message, sizeof(message), "Unsuccessful, wrong password.\n");
            }
            if (errcode == -3) {
                printf("Socket %d unsuccessfully logged into account with name, password: %s, %s\n", fd, users[fd].name, password);
                message_lenght = snprintf(message, sizeof(message), "Unsuccessful, account don't exist.\n");
            }

            if (message_lenght >= sizeof(message)) {
                message_lenght = sizeof(message) - 1;
            }
            if (send_message(fd, &users[fd], poll_struct, message, message_lenght) == -1) {
                perror("send");
            }
        }
        if (action != 0 && action != 1) {
            printf("Socket %d bad action.\n", fd);

            char message[SEND_SIZE];
            unsigned int message_lenght = snprintf(message, sizeof(message), "Don't know what you want to do.\n");
            if (message_lenght >= sizeof(message)) {
                message_lenght = sizeof(message) - 1;
            }
            if (send_message(fd, &users[fd], poll_struct, message, message_lenght) == -1) {
                perror("send");
            }
        }
    }
}