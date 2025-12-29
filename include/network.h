#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H

#include "user.h"
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>

struct Server {
    // socket's info
    int sockfd;
    int nfds;
    struct pollfd *fds;

    // data for connected clients
    struct Client *clients;

    // database
    struct Database *database;
};

struct Client {
    // client part
    int fd;
    int fds_index;
    // input buffer
    char buffer[RECEIVE_SIZE + 1];
    int buffer_size;
    int buffer_checked;
    // output buffer
    char obuffer[SEND_SIZE];
    int obuffer_size;
    int obuffer_sent;
    // connection type
    UserState state;

    // data for chat
    struct User user;
};

/**
 * Extract the IP address from a sockaddr structure, supporting both IPv4 and IPv6.
 * @param sa Pointer to the sockaddr structure (either sockaddr_in or sockaddr_in6).
 * @return A pointer to the network address part (struct in_addr or in6_addr).
 */
void *get_in_addr(struct sockaddr *sa);
/**
 * Configures and creates a listening TCP socket on a specified port.
 * Performs getaddrinfo, socket creation, setsockopt (SO_REUSEADDR), and bind.
 * @param port port
 * @param backlog maximum length of connections
 * @return The listening socket file descriptor, or exits the program on failure.
 */
int get_listener_socket(const char *port, int backlog);
/**
 * Reaps dead (zombie) child processes to prevent system resource leaks.
 * Called automatically when a SIGCHLD signal is received.
 * @param s The signal number (not used directly, but required by signal handler signature).
 */
void sigchld_handler(int s);
/**
 * set socket non blocking
 * @param sockfd socket fd
 * @return execution code
 */
int set_nonblocking(int sockfd);
/**
 * send message without leaks and without socket blocking
 * @param server server struct
 * @param client client struct
 * @param message message
 * @param size message size
 * @return execution code
 */
int send_message(struct Server *server, struct Client *client, char *message, int size);
/**
 * close connection
 * @param server server struct
 * @param client client struct
 */
void close_connection(struct Server *server, struct Client *client);

#endif //TEST_NETWORK_H
