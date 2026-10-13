// src/models/Event.hpp
#pragma once
#include <libpq-fe.h> // Библиотека PostgreSQL
#include <iostream>
#include <string>

class Event {
public:
    // Метод для создания таблицы.
    // Возвращает: 1 — создана, 0 — уже была, -1 — ошибка.
    static int createTable(PGconn* conn) {
        if (!conn) {
            std::cerr << "Connection is null!" << std::endl;
            return -1;
        }

        // Шаг 1. Проверяем наличие таблицы event
        const char* checkSql =
            "SELECT EXISTS ("
            "  SELECT 1 FROM information_schema.tables "
            "  WHERE table_schema = 'public' AND table_name = 'event'"
            ");";

        PGresult* checkRes = PQexec(conn, checkSql);
        if (PQresultStatus(checkRes) != PGRES_TUPLES_OK) {
            std::cerr << "Ошибка проверки таблицы Event: "
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
        // trap_id — внешний ключ на Trap(id) с каскадным удалением.
        // source_ip — 45 символов достаточно для IPv6.
        const char* sql =
            "CREATE TABLE IF NOT EXISTS Event ("
            "id SERIAL PRIMARY KEY, "
            "trap_id INTEGER REFERENCES Trap(id) ON DELETE CASCADE, "
            "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "source_ip VARCHAR(45)"
            ");";

        PGresult* res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Ошибка создания таблицы Event: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return -1;
        }

        PQclear(res);
        return 1;
    }
};