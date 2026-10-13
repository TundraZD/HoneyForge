// src/controllers/Audit_crud.cpp
#include "../models/Audit.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление новой записи аудита
// ============================================================
bool createAudit(PGconn* conn,
    int profile_id,
    int session_id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "INSERT INTO Audit (profile_id, session_id) "
        "VALUES ($1, $2) RETURNING id;";

    std::string profileIdStr = std::to_string(profile_id);
    std::string sessionIdStr = std::to_string(session_id);
    const char* params[2] = {
        profileIdStr.c_str(),
        sessionIdStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 2, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Audit: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) > 0) {
        std::cout << "Запись аудита создана. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение записи аудита по id
// ============================================================
bool readAuditById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, profile_id, session_id FROM Audit WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Audit: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Запись аудита с id=" << id << " не найдена." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | profile_id: " << (PQgetisnull(res, i, 1) ? "NULL" : PQgetvalue(res, i, 1))
            << " | session_id: " << (PQgetisnull(res, i, 2) ? "NULL" : PQgetvalue(res, i, 2))
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех записей аудита
// ============================================================
bool readAllAudits(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "SELECT id, profile_id, session_id FROM Audit ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Audit: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего записей аудита: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | profile_id: " << (PQgetisnull(res, i, 1) ? "NULL" : PQgetvalue(res, i, 1))
            << " | session_id: " << (PQgetisnull(res, i, 2) ? "NULL" : PQgetvalue(res, i, 2))
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление записи аудита по id
// ============================================================
bool updateAudit(PGconn* conn,
    int id,
    int profile_id,
    int session_id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Audit SET profile_id = $1, session_id = $2 WHERE id = $3;";

    std::string profileIdStr = std::to_string(profile_id);
    std::string sessionIdStr = std::to_string(session_id);
    std::string idStr = std::to_string(id);
    const char* params[3] = {
        profileIdStr.c_str(),
        sessionIdStr.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 3, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Audit: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Запись аудита с id=" << id << " не найдена — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Запись аудита id=" << id << " обновлена (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление записи аудита по id
// ============================================================
bool deleteAudit(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Audit WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Audit: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Запись аудита с id=" << id << " не найдена — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Запись аудита id=" << id << " удалена (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}