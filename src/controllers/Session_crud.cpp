// src/controllers/Session_crud.cpp
#include "../models/Session.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление новой сессии
// ============================================================
bool createSession(PGconn* conn,
    int trap_id,
    int event_id,
    const std::string& started_at,
    const std::string& ended_at) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    // ended_at может быть пустым (NULL), если сессия еще активна.
    // Для простоты используем текстовые параметры, PostgreSQL сам преобразует в TIMESTAMP.
    const char* sql =
        "INSERT INTO Session (trap_id, event_id, started_at, ended_at) "
        "VALUES ($1, $2, $3, $4) RETURNING id;";

    std::string trapIdStr = std::to_string(trap_id);
    std::string eventIdStr = std::to_string(event_id);
    const char* params[4] = {
        trapIdStr.c_str(),
        eventIdStr.c_str(),
        started_at.c_str(),
        ended_at.empty() ? nullptr : ended_at.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 4, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Session: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) > 0) {
        std::cout << "Сессия создана. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение сессии по id
// ============================================================
bool readSessionById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, trap_id, event_id, started_at, ended_at FROM Session WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Session: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Сессия с id=" << id << " не найдена." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | trap_id: " << PQgetvalue(res, i, 1)
            << " | event_id: " << PQgetvalue(res, i, 2)
            << " | started_at: " << PQgetvalue(res, i, 3)
            << " | ended_at: " << (PQgetisnull(res, i, 4) ? "NULL" : PQgetvalue(res, i, 4))
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех сессий
// ============================================================
bool readAllSessions(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "SELECT id, trap_id, event_id, started_at, ended_at FROM Session ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Session: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего сессий: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | trap_id: " << PQgetvalue(res, i, 1)
            << " | event_id: " << PQgetvalue(res, i, 2)
            << " | started_at: " << PQgetvalue(res, i, 3)
            << " | ended_at: " << (PQgetisnull(res, i, 4) ? "NULL" : PQgetvalue(res, i, 4))
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление сессии по id
// ============================================================
bool updateSession(PGconn* conn,
    int id,
    int trap_id,
    int event_id,
    const std::string& started_at,
    const std::string& ended_at) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Session SET trap_id = $1, event_id = $2, started_at = $3, ended_at = $4 WHERE id = $5;";

    std::string trapIdStr = std::to_string(trap_id);
    std::string eventIdStr = std::to_string(event_id);
    std::string idStr = std::to_string(id);
    const char* params[5] = {
        trapIdStr.c_str(),
        eventIdStr.c_str(),
        started_at.c_str(),
        ended_at.empty() ? nullptr : ended_at.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 5, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Session: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Сессия с id=" << id << " не найдена — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Сессия id=" << id << " обновлена (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление сессии по id
// ============================================================
bool deleteSession(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Session WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Session: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Сессия с id=" << id << " не найдена — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Сессия id=" << id << " удалена (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}