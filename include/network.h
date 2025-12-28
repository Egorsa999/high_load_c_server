#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H

#include "user.h"
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>

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
 * @param fd socket fd
 * @param user user struct
 * @param poll_struct poll struct
 * @param message message
 * @param size message size
 * @return execution code
 */
int send_message(int fd, struct User *user, struct pollfd *poll_struct, const char *message, int size);

#endif //TEST_NETWORK_H
