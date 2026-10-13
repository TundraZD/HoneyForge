// src/controllers/Honeytoken_crud.cpp
#include "../models/Honeytoken.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление нового honeytoken
// ============================================================
bool createHoneytoken(PGconn* conn,
    int trap_id,
    const std::string& token) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "INSERT INTO Honeytoken (trap_id, token) "
        "VALUES ($1, $2) RETURNING id;";

    std::string trapIdStr = std::to_string(trap_id);
    const char* params[2] = {
        trapIdStr.c_str(),
        token.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 2, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Honeytoken: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) > 0) {
        std::cout << "Honeytoken создан. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение honeytoken по id
// ============================================================
bool readHoneytokenById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, trap_id, token, created_token, triggered_at FROM Honeytoken WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Honeytoken: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Honeytoken с id=" << id << " не найден." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | trap_id: " << PQgetvalue(res, i, 1)
            << " | token: " << PQgetvalue(res, i, 2)
            << " | created_token: " << PQgetvalue(res, i, 3)
            << " | triggered_at: " << (PQgetisnull(res, i, 4) ? "NULL" : PQgetvalue(res, i, 4))
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех honeytokens
// ============================================================
bool readAllHoneytokens(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "SELECT id, trap_id, token, created_token, triggered_at FROM Honeytoken ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Honeytoken: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего honeytokens: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | trap_id: " << PQgetvalue(res, i, 1)
            << " | token: " << PQgetvalue(res, i, 2)
            << " | created_token: " << PQgetvalue(res, i, 3)
            << " | triggered_at: " << (PQgetisnull(res, i, 4) ? "NULL" : PQgetvalue(res, i, 4))
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление honeytoken по id
// ============================================================
bool updateHoneytoken(PGconn* conn,
    int id,
    int trap_id,
    const std::string& token,
    const std::string& triggered_at) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Honeytoken SET trap_id = $1, token = $2, triggered_at = $3 WHERE id = $4;";

    std::string trapIdStr = std::to_string(trap_id);
    std::string idStr = std::to_string(id);
    const char* params[4] = {
        trapIdStr.c_str(),
        token.c_str(),
        triggered_at.empty() ? nullptr : triggered_at.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 4, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Honeytoken: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Honeytoken с id=" << id << " не найден — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Honeytoken id=" << id << " обновлён (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление honeytoken по id
// ============================================================
bool deleteHoneytoken(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Honeytoken WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Honeytoken: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Honeytoken с id=" << id << " не найден — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Honeytoken id=" << id << " удалён (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}