// src/controllers/Artifact_crud.cpp
#include "../models/Artifact.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление нового артефакта
// ============================================================
bool createArtifact(PGconn* conn,
    int event_id,
    const std::string& file_name,
    const std::string& file_path,
    const std::string& file_hash) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "INSERT INTO Artifact (event_id, file_name, file_path, file_hash) "
        "VALUES ($1, $2, $3, $4) RETURNING id;";

    std::string eventIdStr = std::to_string(event_id);
    const char* params[4] = {
        eventIdStr.c_str(),
        file_name.c_str(),
        file_path.c_str(),
        file_hash.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 4, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Artifact: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) > 0) {
        std::cout << "Артефакт создан. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение артефакта по id
// ============================================================
bool readArtifactById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, event_id, file_name, file_path, file_hash, created_at "
        "FROM Artifact WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Artifact: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Артефакт с id=" << id << " не найден." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | event_id: " << PQgetvalue(res, i, 1)
            << " | file_name: " << PQgetvalue(res, i, 2)
            << " | file_path: " << PQgetvalue(res, i, 3)
            << " | file_hash: " << PQgetvalue(res, i, 4)
            << " | created_at: " << PQgetvalue(res, i, 5)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех артефактов
// ============================================================
bool readAllArtifacts(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, event_id, file_name, file_path, file_hash, created_at "
        "FROM Artifact ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Artifact: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего артефактов: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | event_id: " << PQgetvalue(res, i, 1)
            << " | file_name: " << PQgetvalue(res, i, 2)
            << " | file_path: " << PQgetvalue(res, i, 3)
            << " | file_hash: " << PQgetvalue(res, i, 4)
            << " | created_at: " << PQgetvalue(res, i, 5)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление артефакта по id
// ============================================================
bool updateArtifact(PGconn* conn,
    int id,
    int event_id,
    const std::string& file_name,
    const std::string& file_path,
    const std::string& file_hash) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Artifact SET event_id = $1, file_name = $2, file_path = $3, file_hash = $4 "
        "WHERE id = $5;";

    std::string eventIdStr = std::to_string(event_id);
    std::string idStr = std::to_string(id);
    const char* params[5] = {
        eventIdStr.c_str(),
        file_name.c_str(),
        file_path.c_str(),
        file_hash.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 5, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Artifact: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Артефакт с id=" << id << " не найден — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Артефакт id=" << id << " обновлён (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление артефакта по id
// ============================================================
bool deleteArtifact(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Artifact WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Artifact: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Артефакт с id=" << id << " не найден — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Артефакт id=" << id << " удалён (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}