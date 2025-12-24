#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H

#include <sys/socket.h>
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
 * @param backlog maximum lenght of connections
 * @return The listening socket file descriptor, or exits the program on failure.
 */
int get_listener_socket(const char *port, int backlog);
/**
 * Reaps dead (zombie) child processes to prevent system resource leaks.
 * Called automatically when a SIGCHLD signal is received.
 * @param s The signal number (not used directly, but required by signal handler signature).
 */
void sigchld_handler(int s);

#endif //TEST_NETWORK_H
