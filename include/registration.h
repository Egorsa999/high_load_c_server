#ifndef TEST_REGISTRATION_H
#define TEST_REGISTRATION_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#include "user.h"
#include "config.h"

/**
 * registration/login logic
 * @param fd user socket id
 * @param buffer command from user
 * @param amount_bytes size of command
 * @param users users array
 * @param database database
 * @param poll_struct poll struct
 */
void registration(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database, struct pollfd *poll_struct);

#endif //TEST_REGISTRATION_H
