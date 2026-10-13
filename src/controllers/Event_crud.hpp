#pragma once
#include <libpq-fe.h>
#include <string>

bool createEvent(PGconn* conn, int trap_id, const std::string& source_ip);
bool readEventById(PGconn* conn, int id);
bool readAllEvents(PGconn* conn);
bool updateEvent(PGconn* conn, int id, int trap_id, const std::string& source_ip);
bool deleteEvent(PGconn* conn, int id);