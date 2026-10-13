// src/controllers/Trap_crud.cpp
#include "../models/Trap.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление новой ловушки
// ============================================================
bool createTrap(PGconn* conn,
    const std::string& name,
    const std::string& type,
    const std::string& status) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "INSERT INTO Trap (name, type, status) "
        "VALUES ($1, $2, $3) RETURNING id;";

    const char* params[3] = {
        name.c_str(),
        type.c_str(),
        status.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 3, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Trap: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) > 0) {
        std::cout << "Ловушка создана. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение ловушки по id
// ============================================================
bool readTrapById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, name, type, status, created_at FROM Trap WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Trap: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Ловушка с id=" << id << " не найдена." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | name: " << PQgetvalue(res, i, 1)
            << " | type: " << PQgetvalue(res, i, 2)
            << " | status: " << PQgetvalue(res, i, 3)
            << " | created_at: " << PQgetvalue(res, i, 4)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех ловушек
// ============================================================
bool readAllTraps(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "SELECT id, name, type, status, created_at FROM Trap ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Trap: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего ловушек: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | name: " << PQgetvalue(res, i, 1)
            << " | type: " << PQgetvalue(res, i, 2)
            << " | status: " << PQgetvalue(res, i, 3)
            << " | created_at: " << PQgetvalue(res, i, 4)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление ловушки по id
// ============================================================
bool updateTrap(PGconn* conn,
    int id,
    const std::string& name,
    const std::string& type,
    const std::string& status) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Trap SET name = $1, type = $2, status = $3 WHERE id = $4;";

    std::string idStr = std::to_string(id);
    const char* params[4] = {
        name.c_str(),
        type.c_str(),
        status.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 4, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Trap: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Ловушка с id=" << id << " не найдена — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Ловушка id=" << id << " обновлена (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление ловушки по id
// ============================================================
bool deleteTrap(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Trap WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Trap: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Ловушка с id=" << id << " не найдена — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Ловушка id=" << id << " удалена (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}