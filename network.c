#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

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

int get_listener_socket(const char *port, int backlog) {
    int sockfd;
    struct addrinfo hints, *serverinfo, *p;
    struct sigaction sa;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port, &hints, &serverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = serverinfo; p != NULL; p = p -> ai_next) {
        if ((sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p -> ai_addr, p -> ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(serverinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    return sockfd;
}