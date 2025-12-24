#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "network.h"
#include "requests.h"

#define PORT "3490"
#define BACKLOG 32768

int main(void) {
    int sockfd, new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    sqlite3 *database;
    if (init_db(&database) != SQLITE_OK) {
        exit(1);
    }

    struct pollfd fds[BACKLOG + 1];
    struct User users[BACKLOG + 1];
    int nfds = 1;
    int amount_connection = 0;

    fds[0].fd = sockfd = get_listener_socket(PORT, BACKLOG);
    fds[0].events = POLLIN;

    printf("Server started.\n");

    while (1) {
        int poll_amount = poll(fds, nfds, -1);

        if (poll_amount == -1) {
            perror("pull");
            exit(1);
        }

        if (fds[0].revents & POLLIN) {
            sin_size = sizeof(their_addr);
            new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
            } else {
                amount_connection++;

                inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s));

                char message[1024];
                int message_lenght = sprintf(message, "You are %d who connect.\nYou can create account/login into account in format :0/1:name:password:\nWhere 0 it's you want create account, 1 it's you want log into account\n", amount_connection);
                if (send(new_fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }

                fds[nfds].fd = new_fd;
                fds[nfds].events = POLLIN;
                users[new_fd].logged = 0;
                users[new_fd].id = -1;
                printf("Got connection from: %s\nSocket id: %d\n", s, new_fd);
                memset(users[new_fd].name, 0, sizeof(users[new_fd].name));
                memset(users[new_fd].password, 0, sizeof(users[new_fd].password));
                nfds++;
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (!(fds[i].revents & POLLIN)) continue;

            char buffer[1024];
            int amount_bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
            if (amount_bytes <= 0) {
                if (amount_bytes == 0) {
                    printf("Socket %d hung up\n", fds[i].fd);
                } else {
                    perror("recv");
                }
                close(fds[i].fd);
                fds[i] = fds[nfds - 1];
                nfds--;
                i--;
                continue;
            }
            buffer[amount_bytes] = '\0';

            user_request(fds[i].fd, buffer, amount_bytes, users, database, fds, nfds);
        }
    }

    sqlite3_close(database);

    return 0;
}