#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "config.h"
#include "network.h"

void registration(struct Server *server, struct Client *client, char *buffer, int amount_bytes) {
    int action = -1;
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    memset(name, 0, sizeof(name));
    memset(password, 0, sizeof(password));

    //parse command as :type:name:password:
    int argc = sscanf(buffer, ":%d:%" USERNAME_SIZES "[^:]:%" PASSWORD_SIZES "[^:]:", &action, name, password);
    printf("Receive from %d: %s\n", client -> fd, buffer);
    if (argc < 3) {
        printf("Bad format from %d\n", client -> fd);
        char message[SEND_SIZE];
        unsigned int message_length = snprintf(message, sizeof(message), "Bad format.\n");
        if (message_length >= sizeof(message)) {
            message_length = sizeof(message) - 1;
        }
        if (send_message(server, client, message, message_length) == -1) {
            perror("send");
        }
    } else {
        if (action == 0) {
            printf("Socket %d want to create account with name, password: %s, %s\n", client -> fd, name, password);
            strncpy(client -> user.name, name, USERNAME_SIZE - 1);
            strncpy(client -> user.password, password, PASSWORD_SIZE - 1);
            client -> user.name[USERNAME_SIZE - 1] = '\0';
            client -> user.password[PASSWORD_SIZE - 1] = '\0';

            char message[SEND_SIZE];
            unsigned int message_length;
            if (user_save(server -> database, &client -> user) == 0) {
                client -> user.logged = 1;
                printf("Socket %d successfully create account with name, password: %s, %s\n", client -> fd, client -> user.name, password);
                message_length = snprintf(message, sizeof(message), "Successful!\nWelcome %s\n", name);
            } else {
                printf("Socket %d unsuccessfully create account with name, password: %s, %s\n", client -> fd, client -> user.name, password);
                message_length = snprintf(message, sizeof(message), "This name was used, try again.\n");
            }

            if (message_length >= sizeof(message)) {
                message_length = sizeof(message) - 1;
            }
            if (send_message(server, client, message, message_length) == -1) {
                perror("send");
            }
        }
        if (action == 1) {
            printf("Socket %d want to log into account with name, password: %s, %s\n", client -> fd, name, password);
            strncpy(client -> user.name, name, USERNAME_SIZE - 1);
            strncpy(client -> user.password, password, PASSWORD_SIZE - 1);
            client -> user.name[USERNAME_SIZE - 1] = '\0';
            client -> user.password[PASSWORD_SIZE - 1] = '\0';

            int errcode = user_get(server -> database, &client -> user);
            char message[SEND_SIZE];
            unsigned int message_length;
            if (errcode == 0) {
                client -> user.logged = 1;
                printf("Socket %d successfully logged into account with name, password: %s, %s\n", client -> fd, client -> user.name, password);
                message_length = snprintf(message, sizeof(message), "Successful!\nWelcome %s\n", name);
            }
            if (errcode == -2) {
                printf("Socket %d unsuccessfully logged into account with name, password: %s, %s\n", client -> fd, client -> user.name, password);
                message_length = snprintf(message, sizeof(message), "Unsuccessful, wrong password.\n");
            }
            if (errcode == -3) {
                printf("Socket %d unsuccessfully logged into account with name, password: %s, %s\n", client -> fd, client -> user.name, password);
                message_length = snprintf(message, sizeof(message), "Unsuccessful, account don't exist.\n");
            }

            if (message_length >= sizeof(message)) {
                message_length = sizeof(message) - 1;
            }
            if (send_message(server, client, message, message_length) == -1) {
                perror("send");
            }
        }
        if (action != 0 && action != 1) {
            printf("Socket %d bad action.\n", client -> fd);

            char message[SEND_SIZE];
            unsigned int message_length = snprintf(message, sizeof(message), "Don't know what you want to do.\n");
            if (message_length >= sizeof(message)) {
                message_length = sizeof(message) - 1;
            }
            if (send_message(server, client, message, message_length) == -1) {
                perror("send");
            }
        }
    }
}