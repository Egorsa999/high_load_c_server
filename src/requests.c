#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "registration.h"
#include "config.h"

void user_request(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *fds, int nfds) {
    if (users[fd].logged == 0) {
        registration(fd, buffer, amount_bytes, users, database);
    } else {
        char message[SEND_SIZE];
        unsigned int message_lenght;
        // send message to each connected user
        message_lenght = snprintf(message, sizeof(message), "Message from %s: %s\n", users[fd].name, buffer);
        if (message_lenght >= sizeof(message)) {
            message_lenght = sizeof(message) - 1;
        }
        for (int i = 1; i < nfds; i++) {
            if (users[fds[i].fd].logged && users[fds[i].fd].id != users[fd].id) {
                if (send(fds[i].fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            }
        }

        message_lenght = snprintf(message, sizeof(message), "You are logged as %s, wait next updates!\nNow your message was sent to every logged user!\n", users[fd].name);
        if (message_lenght >= sizeof(message)) {
            message_lenght = sizeof(message) - 1;
        }
        if (send(fd, message, message_lenght, 0) == -1) {
            perror("send");
        }
    }
}
