#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "network.h"
#include "user.h"
#include "websocket.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <poll.h>

void sigchld_handler(int s) {
    (void)s;
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void close_connection(struct Server *server, struct Client *client) {
    close(client -> fd);
    int fds_index_close = client -> fds_index;
    memset(&server -> clients[client -> fd], 0, sizeof(struct Client));
    server -> fds[fds_index_close] = server -> fds[server -> nfds - 1];
    server -> clients[server -> fds[fds_index_close].fd].fds_index = fds_index_close;
    server -> nfds--;
}

int send_message(struct Server *server, struct Client *client, char *message, int size) {
    char *imessage = message;
    if (client -> state == PROTO_WS_CONNECTED) {
        char frame[SEND_SIZE];
        memcpy(frame, message, size);
        if (text_to_frame(frame, &size) == -1) {
            return -1; // buffer overflow
        }
        imessage = frame;
    }
    // check if buffer not empty
    if (client -> obuffer_size) {
        if (client -> obuffer_size + size > SEND_SIZE) {
            return -1; // buffer overflow
        }

        memcpy(client -> obuffer + client -> obuffer_size, imessage, size);
        client -> obuffer_size += size;
        server -> fds[client -> fds_index].events |= POLLOUT;
        return 0;
    }

    int sent = send(client -> fd, imessage, size, 0);

    if (sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            sent = 0;
        } else {
            perror("send");
            return -1;
        }
    }

    // if we don't sent all
    if (sent < size) {
        int remaining = size - sent;
        if (remaining > SEND_SIZE) return -1;

        memcpy(client -> obuffer, imessage + sent, remaining);
        client -> obuffer_size = remaining;
        client -> obuffer_sent = 0;
        server -> fds[client -> fds_index].events |= POLLOUT;
    }

    return 0;
}

int set_nonblocking(int sockfd) {
    // get flags which exist now
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) return -1;

    // add nonblock flag
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

int get_listener_socket(const char *port, int backlog) {
    int sockfd;
    struct addrinfo hints, *serverinfo, *p;
    struct sigaction sa;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof(hints));
    // port search criteria
    hints.ai_family = AF_INET; // use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP flow
    hints.ai_flags = AI_PASSIVE; // use my local IP

    // get all available addresses
    if ((rv = getaddrinfo(NULL, port, &hints, &serverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // try bind socket
    for (p = serverinfo; p != NULL; p = p -> ai_next) {
        // try to create socket
        if ((sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        // for using our port after server reboot
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        // connect socket to our port
        if (bind(sockfd, p -> ai_addr, p -> ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(serverinfo); // clear memory after getaddrinfo

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // set nonblocking
    if (set_nonblocking(sockfd) == -1) {
        perror("set_nonblocking listener");
        exit(1);
    }

    // put socket into listener mode
    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(1);
    }

    // setting up a signal handler for clearing zombie-process, if we will use fork() or equivalent
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    return sockfd;
}