#ifndef TEST_REQUESTS_H
#define TEST_REQUESTS_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "registration.h"

/**
 * process users commands
 * @param fd user socket id
 * @param buffer command from user
 * @param amount_bytes size of command
 * @param users users array
 * @param database database
 * @param fds listener socket id
 * @param nfds array of sockets
 */
void user_request(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *fds, int nfds);

#endif //TEST_REQUESTS_H
