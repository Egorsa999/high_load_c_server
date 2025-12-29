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

    // database init
    struct Database database;
    if (init_db(&database) != SQLITE_OK) {
        exit(1);
    }

    struct pollfd *fds = calloc(BACKLOG + 1, sizeof(struct pollfd)); // socket's array
    struct Client *clients = calloc(BACKLOG + 1, sizeof(struct Client)); // client's array
    if (fds == NULL || clients == NULL) {
        printf("Not enough memory\n");
        return 0;
    }
    int amount_connection = 0; // count of all connections

    fds[0].fd = sockfd = get_listener_socket(PORT, BACKLOG); // get listener descriptor
    fds[0].events = POLLIN; // ready to listen

    // init server struct
    struct Server server;
    server.sockfd = sockfd;
    server.nfds = 1;
    server.database = &database;
    server.fds = fds;
    server.clients = clients;

    printf("Server started.\n");

    while (1) {

        // wait update in some socket
        int poll_amount = poll(fds, server.nfds, -1);

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
                fds[server.nfds].fd = new_fd;
                fds[server.nfds].events = POLLIN;

                // init client as not logged
                memset(&clients[new_fd], 0, sizeof(struct Client));
                clients[new_fd].fd = new_fd;
                clients[new_fd].fds_index = server.nfds;
                clients[new_fd].state = PROTO_UNKNOWN;
                clients[new_fd].user.logged = 0;
                clients[new_fd].user.id = -1;
                server.nfds++;

                printf("Got connection from: %s\nSocket id: %d\n", s, new_fd);
            }
        }

        // check each client socket
        for (int i = 1; i < server.nfds; i++) {
            if (fds[i].revents & POLLIN) {
                struct Client *client = &clients[fds[i].fd];

                int remaining_size = RECEIVE_SIZE - client -> buffer_size;
                printf("%d, %d\n", fds[i].fd, remaining_size);
                if (remaining_size <= 0) {
                    close_connection(&server, client);
                    i--;
                    continue;
                }
                int amount_bytes = recv(fds[i].fd, client -> buffer + client -> buffer_size, remaining_size, 0);
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
                    close_connection(&server, client);
                    i--;
                    continue;
                }

                client -> buffer_size += amount_bytes;

                // determine it's TCP or WebSocket w
                if (client -> state == PROTO_UNKNOWN) {
                    if (client -> buffer_size >= 3) {
                        if (strncmp(client -> buffer, "GET", 3) == 0) {
                            client -> state = PROTO_WS_HANDSHAKE;
                        } else {
                            client -> state = PROTO_TCP;
                        }
                    } else {
                        continue;
                    }
                }

                if (client -> state == PROTO_TCP) {
                    char *ptr;
                    int shift_size = 0;
                    char *last = client -> buffer;

                    // find separator
                    while ((ptr = memchr(client -> buffer + client -> buffer_checked, CMD_SEP, client -> buffer_size - client -> buffer_checked))) {
                        int cmd_size = ptr - last + 1;
                        *ptr = '\0';

                        printf("Found command: %s\n", last);
                        // processing command
                        user_request(&server, client, last, cmd_size);

                        last = ptr + 1;
                        client -> buffer_checked = last - client -> buffer;
                    }

                    //shift if we do some commands
                    if (last != client -> buffer) {
                        remaining_size = client -> buffer_size - (last - client -> buffer);
                        if (remaining_size > 0) {
                            memmove(client -> buffer, last, remaining_size);
                        }
                        client -> buffer_size = remaining_size;
                    }

                    client -> buffer_checked = client -> buffer_size;
                } else {
                    if (client -> state == PROTO_WS_HANDSHAKE) {
                        client -> buffer[client -> buffer_size] = '\0';
                        // start search from -3 bytes because length string is 4
                        char *ptr = strstr(client -> buffer + (client -> buffer_checked >= 3 ? client -> buffer_checked - 3 : 0), "\r\n\r\n");
                        if (ptr) {
                            if (ws_handshake(&server, client) == 0) {
                                client -> state = PROTO_WS_CONNECTED;
                                int header_length = (ptr - client -> buffer) + 4;
                                int remaining = client -> buffer_size - header_length;

                                //clearing buffer
                                if (remaining > 0) {
                                    memmove(client -> buffer, client -> buffer + header_length, remaining);
                                }
                                client -> buffer_size = remaining;
                                client -> buffer_checked = 0;

                                printf("Handshake done correct for socket %d\n", fds[i].fd);
                            } else {
                                printf("Handshake failed for socket %d\n", fds[i].fd);
                                close_connection(&server, client);
                                i--;
                                continue;
                            }
                        } else {
                            client -> buffer_checked = client -> buffer_size;
                        }
                    } else {
                        if (client -> state == PROTO_WS_CONNECTED) {
                            int err_code;
                            int was_checked = client -> buffer_checked;
                            while ((err_code = frame_to_text(client)) > 0) {
                                // process command which we receive
                                user_request(&server, client, client -> buffer + was_checked, err_code);
                                was_checked = client -> buffer_checked;
                            }
                            // close connection
                            if (err_code == -1) {
                                printf("WebSocket %d disconnect\n", fds[i].fd);
                                close_connection(&server, client);
                                i--;
                                continue;
                            }
                            // buffer shift
                            remaining_size = client -> buffer_size - client -> buffer_checked;
                            if (remaining_size > 0) {
                                memmove(client -> buffer, client -> buffer + client -> buffer_checked, remaining_size);
                            }
                            client -> buffer_size = remaining_size;
                            client -> buffer_checked = 0;
                        } else {
                            printf("Unknown state.\n");
                            close_connection(&server, client);
                            i--;
                            continue;
                        }
                    }
                }
            }

            if (fds[i].revents & POLLOUT) {
                struct Client *client = &clients[fds[i].fd];
                int remaining = client -> obuffer_size - client -> obuffer_sent;

                // try send all what we have
                int sent = send(fds[i].fd, client -> obuffer + client -> obuffer_sent, remaining, 0);

                if (sent > 0) {
                    client -> obuffer_sent += sent;
                    // if we sent all
                    if (client -> obuffer_sent == client -> obuffer_size) {
                        client -> obuffer_sent = 0;
                        client -> obuffer_size = 0;

                        fds[i].events &= ~POLLOUT;
                    }
                } else {
                    if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                        // socket error
                        close_connection(&server, client);
                        i--;
                        continue;
                    }
                }
            }
        }
    }

    sqlite3_close(database.database);
    free(clients);
    free(fds);

    return 0;
}