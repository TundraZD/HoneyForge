#pragma once
#include <libpq-fe.h>
#include <string>

bool createProfile(PGconn* conn, const std::string& login,
    const std::string& password_hash, const std::string& name);
bool readProfileById(PGconn* conn, int id);
bool readAllProfiles(PGconn* conn);
bool updateProfile(PGconn* conn, int id, const std::string& login,
    const std::string& password_hash, const std::string& name);
bool deleteProfile(PGconn* conn, int id);