// src/controllers/Profile_crud.cpp
#include "../models/Profile.hpp"
#include <string>
#include <iostream>
#include <libpq-fe.h>

// ============================================================
// CREATE — добавление нового профиля
// ============================================================
bool createProfile(PGconn* conn,
    const std::string& login,
    const std::string& password_hash,
    const std::string& name) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    // Параметризованный запрос — защита от SQL-инъекций
    const char* sql =
        "INSERT INTO Profile (login, password_hash, name) "
        "VALUES ($1, $2, $3) RETURNING id;";

    const char* params[3] = {
        login.c_str(),
        password_hash.c_str(),
        name.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        sql,
        3,          // количество параметров
        nullptr,    // типы параметров (пусть определяются автоматически)
        params,     // значения
        nullptr,    // длины (для текстового формата не нужны)
        nullptr,    // форматы (0 = текст)
        0           // результат в текстовом формате
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка INSERT Profile: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    // Получаем сгенерированный id
    if (PQntuples(res) > 0) {
        std::cout << "Профиль создан. id = " << PQgetvalue(res, 0, 0) << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ — получение профиля по id
// ============================================================
bool readProfileById(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "SELECT id, login, password_hash, name FROM Profile WHERE id = $1;";

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params,
        nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT Profile: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    if (rows == 0) {
        std::cout << "Профиль с id=" << id << " не найден." << std::endl;
        PQclear(res);
        return false;
    }

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | login: " << PQgetvalue(res, i, 1)
            << " | password_hash: " << PQgetvalue(res, i, 2)
            << " | name: " << PQgetvalue(res, i, 3)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// READ ALL — получение всех профилей
// ============================================================
bool readAllProfiles(PGconn* conn) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "SELECT id, login, password_hash, name FROM Profile ORDER BY id;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка SELECT ALL Profile: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    int rows = PQntuples(res);
    std::cout << "Всего профилей: " << rows << std::endl;

    for (int i = 0; i < rows; ++i) {
        std::cout << "id: " << PQgetvalue(res, i, 0)
            << " | login: " << PQgetvalue(res, i, 1)
            << " | password_hash: " << PQgetvalue(res, i, 2)
            << " | name: " << PQgetvalue(res, i, 3)
            << std::endl;
    }

    PQclear(res);
    return true;
}

// ============================================================
// UPDATE — обновление профиля по id
// ============================================================
bool updateProfile(PGconn* conn,
    int id,
    const std::string& login,
    const std::string& password_hash,
    const std::string& name) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql =
        "UPDATE Profile SET login = $1, password_hash = $2, name = $3 WHERE id = $4;";

    std::string idStr = std::to_string(id);
    const char* params[4] = {
        login.c_str(),
        password_hash.c_str(),
        name.c_str(),
        idStr.c_str()
    };

    PGresult* res = PQexecParams(conn, sql, 4, nullptr, params,
        nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка UPDATE Profile: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    // Проверяем, была ли обновлена хотя бы одна строка
    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Профиль с id=" << id << " не найден — обновление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Профиль id=" << id << " обновлён (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}

// ============================================================
// DELETE — удаление профиля по id
// ============================================================
bool deleteProfile(PGconn* conn, int id) {
    if (!conn) {
        std::cerr << "Connection is null!" << std::endl;
        return false;
    }

    const char* sql = "DELETE FROM Profile WHERE id = $1;";
    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, nullptr, params,
        nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка DELETE Profile: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    std::string affected = PQcmdTuples(res);
    if (affected == "0") {
        std::cout << "Профиль с id=" << id << " не найден — удаление не выполнено." << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Профиль id=" << id << " удалён (строк: " << affected << ")." << std::endl;
    PQclear(res);
    return true;
}