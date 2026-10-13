// src/models/Audit.hpp
#pragma once
#include <libpq-fe.h> // Библиотека PostgreSQL
#include <iostream>
#include <string>

class Audit {
public:
    // Метод для создания таблицы.
    // Возвращает: 1 — создана, 0 — уже была, -1 — ошибка.
    static int createTable(PGconn* conn) {
        if (!conn) {
            std::cerr << "Connection is null!" << std::endl;
            return -1;
        }

        // Шаг 1. Проверяем наличие таблицы audit
        const char* checkSql =
            "SELECT EXISTS ("
            "  SELECT 1 FROM information_schema.tables "
            "  WHERE table_schema = 'public' AND table_name = 'audit'"
            ");";

        PGresult* checkRes = PQexec(conn, checkSql);
        if (PQresultStatus(checkRes) != PGRES_TUPLES_OK) {
            std::cerr << "Ошибка проверки таблицы Audit: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(checkRes);
            return -1;
        }

        bool existed = (std::string(PQgetvalue(checkRes, 0, 0)) == "t");
        PQclear(checkRes);
        if (existed) {
            return 0;
        }

        // Шаг 2. Создаём таблицу.
        // Ссылается на Profile(id) и Session(id).
        // ON DELETE SET NULL: если профиль или сессия удалены,
        // поле обнуляется, а запись аудита сохраняется.
        const char* sql =
            "CREATE TABLE IF NOT EXISTS Audit ("
            "id SERIAL PRIMARY KEY, "
            "profile_id INTEGER REFERENCES Profile(id) ON DELETE SET NULL, "
            "session_id INTEGER REFERENCES Session(id) ON DELETE SET NULL"
            ");";

        PGresult* res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Ошибка создания таблицы Audit: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return -1;
        }

        PQclear(res);
        return 1;
    }
};