#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#include "user.h"
#include "network.h"
#include "requests.h"
#include "config.h"
#include "websocket.h"

int main(void) {
    //for logs in docker
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // for safe crush
    signal(SIGPIPE, SIG_IGN);

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
    if (fds == NULL || users == NULL) {
        printf("Not enough memory\n");
        return 0;
    }
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
                //set nonblocking
                if (set_nonblocking(new_fd) == -1) {
                    close(new_fd);
                    perror("set_nonblocking");
                    continue;
                }

                // client ip-address convert
                inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s));

                //if socket too large
                if (new_fd >= BACKLOG) {
                    close(new_fd);
                    printf("Socket %d too large\n", new_fd);
                    continue;
                }

                // add new socket for client
                fds[nfds].fd = new_fd;
                fds[nfds].events = POLLIN;

                // init user as not logged
                memset(&users[new_fd], 0, sizeof(struct User));
                users[new_fd].state = PROTO_UNKNOWN;
                users[new_fd].logged = 0;
                users[new_fd].id = -1;
                nfds++;

                printf("Got connection from: %s\nSocket id: %d\n", s, new_fd);
            }
        }

        // check each client socket
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                struct User *user = &users[fds[i].fd];

                int remaining_size = RECEIVE_SIZE - user -> buffer_size;
                int amount_bytes = recv(fds[i].fd, user -> buffer + user -> buffer_size, remaining_size, 0);
                if (amount_bytes <= 0) {
                    if (amount_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                        continue; // socket still alive
                    }
                    // client close connection or flood or some error
                    if (amount_bytes == 0) {
                        printf("Socket %d hung up or flood\n", fds[i].fd);
                    } else {
                        perror("recv");
                    }
                    memset(&users[fds[i].fd], 0, sizeof(struct User));
                    close(fds[i].fd);

                    // delete socket
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                    continue;
                }

                user -> buffer_size += amount_bytes;

                // determine it's TCP or WebSocket
                if (user -> state == PROTO_UNKNOWN) {
                    if (user -> buffer_size >= 3) {
                        if (strncmp(user -> buffer, "GET", 3) == 0) {
                            user -> state = PROTO_WS_HANDSHAKE;
                        } else {
                            user -> state = PROTO_TCP;
                        }
                    } else {
                        continue;
                    }
                }

                if (user -> state == PROTO_TCP) {
                    char *ptr;
                    int shift_size = 0;
                    char *last = user -> buffer;

                    // find separator
                    while ((ptr = memchr(user -> buffer + user -> buffer_checked, CMD_SEP, user -> buffer_size - user -> buffer_checked))) {
                        int cmd_size = ptr - last + 1;
                        *ptr = '\0';

                        printf("Found command: %s\n", last);
                        // processing command
                        user_request(fds[i].fd, last, cmd_size, users, database, fds, nfds, &fds[i]);

                        last = ptr + 1;
                        user -> buffer_checked = last - user -> buffer;
                    }

                    //shift if we do some commands
                    if (last != user -> buffer) {
                        remaining_size = user -> buffer_size - (last - user -> buffer);
                        if (remaining_size > 0) {
                            memmove(user -> buffer, last, remaining_size);
                        }
                        user -> buffer_size = remaining_size;
                    }

                    user -> buffer_checked = user -> buffer_size;
                } else {
                    if (user -> state == PROTO_WS_HANDSHAKE) {
                        user -> buffer[user -> buffer_size] = '\0';
                        // start search from -3 bytes because length string is 4
                        char *ptr = strstr(user -> buffer + (user -> buffer_checked >= 3 ? user -> buffer_checked - 3 : 0), "\r\n\r\n");
                        if (ptr) {
                            if (ws_handshake(fds[i].fd, user, &fds[i]) == 0) {
                                user -> state = PROTO_WS_CONNECTED;
                                int header_length = (ptr - user -> buffer) + 4;
                                int remaining = user -> buffer_size - header_length;

                                //clearing buffer
                                if (remaining > 0) {
                                    memmove(user -> buffer, user -> buffer + header_length, remaining);
                                }
                                user -> buffer_size = remaining;
                                user -> buffer_checked = 0;

                                printf("Handshake done correct for socket %d\n", fds[i].fd);
                            } else {
                                printf("Handshake failed for socket %d\n", fds[i].fd);
                                close(fds[i].fd);
                                memset(&users[fds[i].fd], 0, sizeof(struct User));
                                fds[i] = fds[nfds - 1];
                                nfds--;
                                i--;
                                continue;
                            }
                        } else {
                            user -> buffer_checked = user -> buffer_size;
                        }
                    } else {
                        if (user -> state == PROTO_WS_CONNECTED) {
                            int err_code;
                            int was_checked = user -> buffer_checked;
                            while ((err_code = frame_to_text(user)) > 0) {
                                // process command which we receive
                                user_request(fds[i].fd, user -> buffer + was_checked, err_code, users, database, fds, nfds, &fds[i]);
                                was_checked = user -> buffer_checked;
                            }
                            // close connection
                            if (err_code == -1) {
                                printf("WebSocket %d disconnect\n", fds[i].fd);
                                close(fds[i].fd);
                                memset(&users[fds[i].fd], 0, sizeof(struct User));
                                fds[i] = fds[nfds - 1];
                                nfds--;
                                i--;
                                continue;
                            }
                            // buffer shift
                            remaining_size = user -> buffer_size - user -> buffer_checked;
                            if (remaining_size > 0) {
                                memmove(user -> buffer, user -> buffer + user -> buffer_checked, remaining_size);
                            }
                            user -> buffer_size = remaining_size;
                            user -> buffer_checked = 0;
                        } else {
                            printf("Unknown state.\n");
                            close(fds[i].fd);
                            memset(&users[fds[i].fd], 0, sizeof(struct User));
                            fds[i] = fds[nfds - 1];
                            nfds--;
                            i--;
                            continue;
                        }
                    }
                }
            }

            if (fds[i].revents & POLLOUT) {
                struct User *user = &users[fds[i].fd];
                int remaining = user -> obuffer_size - user -> obuffer_sent;

                // try send all what we have
                int sent = send(fds[i].fd, user -> obuffer + user -> obuffer_sent, remaining, 0);

                if (sent > 0) {
                    user -> obuffer_sent += sent;
                    // if we sent all
                    if (user -> obuffer_sent == user -> obuffer_size) {
                        user -> obuffer_sent = 0;
                        user -> obuffer_size = 0;

                        fds[i].events &= ~POLLOUT;
                    }
                } else {
                    if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                        // socket error
                        perror("send");
                        close(fds[i].fd);
                        memset(&users[fds[i].fd], 0, sizeof(struct User));
                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                        continue;
                    }
                }
            }
        }
    }

    sqlite3_close(database);
    free(users);
    free(fds);

    return 0;
}