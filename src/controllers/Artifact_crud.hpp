#pragma once
#include <libpq-fe.h>
#include <string>

bool createArtifact(PGconn* conn, int event_id, const std::string& file_name,
    const std::string& file_path, const std::string& file_hash);
bool readArtifactById(PGconn* conn, int id);
bool readAllArtifacts(PGconn* conn);
bool updateArtifact(PGconn* conn, int id, int event_id,
    const std::string& file_name, const std::string& file_path,
    const std::string& file_hash);
bool deleteArtifact(PGconn* conn, int id);