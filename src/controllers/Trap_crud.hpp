#pragma once
#include <libpq-fe.h>
#include <string>

bool createTrap(PGconn* conn, const std::string& name,
    const std::string& type, const std::string& status);
bool readTrapById(PGconn* conn, int id);
bool readAllTraps(PGconn* conn);
bool updateTrap(PGconn* conn, int id, const std::string& name,
    const std::string& type, const std::string& status);
bool deleteTrap(PGconn* conn, int id);