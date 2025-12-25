#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "registration.h"

void user_request(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *fds, int nfds) {
    if (users[fd].logged == 0) {
        registration(fd, buffer, amount_bytes, users, database);
    } else {
        char message[1024];
        // send message to each connected user
        for (int i = 1; i < nfds; i++) {
            if (users[fds[i].fd].logged && users[fds[i].fd].id != users[fd].id) {
                int message_lenght = sprintf(message, "Message from %s: %s", users[fd].name, buffer);
                if (send(fds[i].fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }
            }
        }

        int message_lenght = sprintf(message, "You are logged as %s, wait next updates!\nNow your message was sent to every logged user!\n", users[fd].name);
        if (send(fd, message, message_lenght, 0) == -1) {
            perror("send");
        }
    }
}
