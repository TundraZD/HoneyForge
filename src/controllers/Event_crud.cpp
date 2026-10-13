// src/controllers/Event_crud.cpp
#include "../models/Event.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление нового события
// ============================================================
bool createEvent(PGconn* conn,
    int trap_id,
    const std::string& source_ip) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "INSERT INTO Event (trap_id, source_ip) "
        "VALUES ($1, $2) RETURNING id;";

    std::string trapIdStr = std::to_string(trap_id);
    const char* params[2] = {
        trapIdStr.c_str(),
        source_ip.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 2, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Event: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) > 0) {
        std::cout << "Событие создано. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение события по id
// ============================================================
bool readEventById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, trap_id, timestamp, source_ip FROM Event WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Event: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Событие с id=" << id << " не найдено." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | trap_id: " << PQgetvalue(res, i, 1)
            << " | timestamp: " << PQgetvalue(res, i, 2)
            << " | source_ip: " << PQgetvalue(res, i, 3)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех событий
// ============================================================
bool readAllEvents(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "SELECT id, trap_id, timestamp, source_ip FROM Event ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Event: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего событий: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | trap_id: " << PQgetvalue(res, i, 1)
            << " | timestamp: " << PQgetvalue(res, i, 2)
            << " | source_ip: " << PQgetvalue(res, i, 3)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление события по id
// ============================================================
bool updateEvent(PGconn* conn,
    int id,
    int trap_id,
    const std::string& source_ip) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Event SET trap_id = $1, source_ip = $2 WHERE id = $3;";

    std::string trapIdStr = std::to_string(trap_id);
    std::string idStr = std::to_string(id);
    const char* params[3] = {
        trapIdStr.c_str(),
        source_ip.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 3, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Event: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Событие с id=" << id << " не найдено — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Событие id=" << id << " обновлено (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление события по id
// ============================================================
bool deleteEvent(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Event WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Event: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Событие с id=" << id << " не найдено — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Событие id=" << id << " удалено (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}