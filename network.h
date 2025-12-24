#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H

#include <sys/socket.h>
#include <netdb.h>

void *get_in_addr(struct sockaddr *sa);
int get_listener_socket(const char *port, int backlog);
void sigchld_handler(int s);

#endif //TEST_NETWORK_H
