#pragma once
#include <libpq-fe.h>
#include <string>

bool createSession(PGconn* conn, int trap_id, int event_id,
    const std::string& started_at, const std::string& ended_at);
bool readSessionById(PGconn* conn, int id);
bool readAllSessions(PGconn* conn);
bool updateSession(PGconn* conn, int id, int trap_id, int event_id,
    const std::string& started_at, const std::string& ended_at);
bool deleteSession(PGconn* conn, int id);