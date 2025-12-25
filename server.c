#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "network.h"
#include "requests.h"

#define PORT "3490" // port for listening socket
#define BACKLOG 32768 // max amount of simultaneous connections

int main(void) {
    //for logs in docker
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    int sockfd, new_fd;
    struct sockaddr_storage their_addr; // information about connected client
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    //database init
    sqlite3 *database;
    if (init_db(&database) != SQLITE_OK) {
        exit(1);
    }

    struct pollfd *fds = calloc(BACKLOG + 1, sizeof(struct pollfd)); // socket's array
    struct User *users = calloc(BACKLOG + 1, sizeof(struct User)); // user's array
    int nfds = 1; // amount of connections
    int amount_connection = 0; // count of all connections

    fds[0].fd = sockfd = get_listener_socket(PORT, BACKLOG); // get listener descriptor
    fds[0].events = POLLIN; // ready to listen

    printf("Server started.\n");

    while (1) {

        // wait update in some socket
        int poll_amount = poll(fds, nfds, -1);

        if (poll_amount == -1) {
            perror("poll");
            exit(1);
        }

        // check new connection
        if (fds[0].revents & POLLIN) {
            sin_size = sizeof(their_addr);
            new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
            } else {
                amount_connection++;

                // client ip-address convert
                inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s));

                char message[1024];
                int message_lenght = sprintf(message, "You are %d who connect.\nYou can create account/login into account in format :0/1:name:password:\nWhere 0 it's you want create account, 1 it's you want log into account\n", amount_connection);
                if (send(new_fd, message, message_lenght, 0) == -1) {
                    perror("send");
                }

                // add new socket for client
                fds[nfds].fd = new_fd;
                fds[nfds].events = POLLIN;

                // init user as not logged
                users[new_fd].logged = 0;
                users[new_fd].id = -1;
                printf("Got connection from: %s\nSocket id: %d\n", s, new_fd);
                memset(users[new_fd].name, 0, sizeof(users[new_fd].name));
                memset(users[new_fd].password, 0, sizeof(users[new_fd].password));
                nfds++;
            }
        }

        // check each client socket
        for (int i = 1; i < nfds; i++) {
            if (!(fds[i].revents & POLLIN)) continue;

            char buffer[1024];
            int amount_bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
            if (amount_bytes <= 0) {
                // client close connection or some error
                if (amount_bytes == 0) {
                    printf("Socket %d hung up\n", fds[i].fd);
                } else {
                    perror("recv");
                }
                close(fds[i].fd);

                // delete socket
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