#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "registration.h"
#include "config.h"
#include "network.h"

void user_request(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *fds, int nfds, struct pollfd *poll_struct) {
    if (users[fd].state == PROTO_WS_CONNECTED) {
        printf("Receive from WebSocket %d: %s\n", fd, buffer);
    } else {
        printf("Receive from TCP %d: %s\n", fd, buffer);
    }
    if (users[fd].logged == 0) {
        registration(fd, buffer, amount_bytes, users, database, poll_struct);
    } else {
        char message[SEND_SIZE];
        unsigned int message_length;
        // send message to each connected user
        message_length = snprintf(message, sizeof(message), "Message from %s: %s\n", users[fd].name, buffer);
        if (message_length >= sizeof(message)) {
            message_length = sizeof(message) - 1;
        }
        for (int i = 1; i < nfds; i++) {
            if (users[fds[i].fd].logged && users[fds[i].fd].id != users[fd].id) {
                if (send_message(fds[i].fd, &users[fds[i].fd], &fds[i], message, message_length) == -1) {
                    perror("send");
                }
            }
        }

        message_length = snprintf(message, sizeof(message), "You are logged as %s, wait next updates!\nNow your message was sent to every logged user!\n", users[fd].name);
        if (message_length >= sizeof(message)) {
            message_length = sizeof(message) - 1;
        }
        if (send_message(fd, &users[fd], poll_struct, message, message_length) == -1) {
            perror("send");
        }
    }
}
