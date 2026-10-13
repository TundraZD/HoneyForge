#pragma once
#include <libpq-fe.h>
#include <string>

bool createHoneytoken(PGconn* conn, int trap_id, const std::string& token);
bool readHoneytokenById(PGconn* conn, int id);
bool readAllHoneytokens(PGconn* conn);
bool updateHoneytoken(PGconn* conn, int id, int trap_id,
    const std::string& token, const std::string& triggered_at);
bool deleteHoneytoken(PGconn* conn, int id);