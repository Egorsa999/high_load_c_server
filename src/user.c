#include "user.h"
#include <string.h>
#include <stdio.h>

int init_db(struct Database *database) {
    //open or create database file
    int return_error = sqlite3_open("data/server.db", &database -> database);
    if (return_error != SQLITE_OK) return return_error;

    //sql-query
    char *sql = "CREATE TABLE IF NOT EXISTS Users ("
                "Id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "Name TEXT UNIQUE, "
                "Password TEXT);";

    char *err_msg = 0;
    //create database request
    return_error = sqlite3_exec(database -> database, sql, 0, 0, &err_msg);
    if (return_error != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return return_error;
    }
    return SQLITE_OK;
}

int user_save(struct Database *database, struct User *user) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Users (Name, Password) VALUES (?, ?);";

    if (sqlite3_prepare_v2(database -> database, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    //connection user data to '?' in request
    sqlite3_bind_text(stmt, 1, user -> name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user -> password, -1, SQLITE_STATIC);

    //request executing
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        //get user id from database
        user -> id = (int)sqlite3_last_insert_rowid(database -> database);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return -1;
}

int user_get(struct Database *database, struct User *user) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Id, Password FROM Users WHERE Name = ?;";
    int status;

    if (sqlite3_prepare_v2(database -> database, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, user -> name, -1, SQLITE_STATIC);

    //if we find user
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *db_pass = (const char *)sqlite3_column_text(stmt, 1);

        //check password
        if (db_pass && strcmp(user -> password, db_pass) == 0) {
            user -> id = sqlite3_column_int(stmt, 0); // get id from database
            user -> logged = 1; // mark user as logged
            status = 0; // OK
        } else {
            status = -2; // bad password
        }
    } else {
        status = -3; // user don't exist
    }

    sqlite3_finalize(stmt);
    return status;
}