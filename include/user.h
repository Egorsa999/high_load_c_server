#ifndef TEST_USER_H
#define TEST_USER_H

#include <sqlite3.h>
#include <sys/epoll.h>

#include "config.h"

struct User {
    // chat part
    int logged;
    int id;
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
};

struct Database {
    sqlite3 *database;
};

/**
 * database init
 * @param database database
 * @return execution result
 */
int init_db(struct Database *database);
/**
 * save user in database
 * @param database database
 * @param user users array
 * @return execution result
 */
int user_save(struct Database *database, struct User *user);
/**
 * get user from database
 * @param database database
 * @param user user
 * @return execution result
 */
int user_get(struct Database *database, struct User *user);

#endif //TEST_USER_H
