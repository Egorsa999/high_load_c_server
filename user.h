#ifndef TEST_USER_H
#define TEST_USER_H

#include <sqlite3.h>

struct User {
    int logged;
    int id;
    char name[51];
    char password[31];
};

/**
 * database init
 * @param db database
 * @return execution result
 */
int init_db(sqlite3 **db);
/**
 * save user in database
 * @param database database
 * @param user users array
 * @return execution result
 */
int user_save(sqlite3 *database, struct User *user);
/**
 * get user from database
 * @param database database
 * @param user user
 * @return execution result
 */
int user_get(sqlite3 *database, struct User *user);

#endif //TEST_USER_H
