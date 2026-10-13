#pragma once
#include <libpq-fe.h>

bool createAudit(PGconn* conn, int profile_id, int session_id);
bool readAuditById(PGconn* conn, int id);
bool readAllAudits(PGconn* conn);
bool updateAudit(PGconn* conn, int id, int profile_id, int session_id);
bool deleteAudit(PGconn* conn, int id);