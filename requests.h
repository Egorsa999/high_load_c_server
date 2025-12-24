#ifndef TEST_REQUESTS_H
#define TEST_REQUESTS_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "registration.h"

void user_request(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *fds, int nfds);

#endif //TEST_REQUESTS_H
