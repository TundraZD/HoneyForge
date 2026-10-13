// src/models/Trap.hpp
#pragma once
#include <libpq-fe.h> // Библиотека PostgreSQL
#include <iostream>
#include <string>

class Trap {
public:
    // Метод для создания таблицы.
    // Возвращает: 1 — создана, 0 — уже была, -1 — ошибка.
    static int createTable(PGconn* conn) {
        if (!conn) {
            std::cerr << "Connection is null!" << std::endl;
            return -1;
        }

        // Шаг 1. Проверяем, существует ли таблица trap в схеме public.
        const char* checkSql =
            "SELECT EXISTS ("
            "  SELECT 1 FROM information_schema.tables "
            "  WHERE table_schema = 'public' AND table_name = 'trap'"
            ");";

        PGresult* checkRes = PQexec(conn, checkSql);
        if (PQresultStatus(checkRes) != PGRES_TUPLES_OK) {
            std::cerr << "Ошибка проверки таблицы Trap: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(checkRes);
            return -1;
        }

        // Если таблица уже есть — выходим без создания
        bool existed = (std::string(PQgetvalue(checkRes, 0, 0)) == "t");
        PQclear(checkRes);
        if (existed) {
            return 0;
        }

        // Шаг 2. Создаём таблицу.
        // created_at по умолчанию — текущее время
        const char* sql =
            "CREATE TABLE IF NOT EXISTS Trap ("
            "id SERIAL PRIMARY KEY, "
            "name VARCHAR(100) NOT NULL, "
            "type VARCHAR(50), "
            "status VARCHAR(20), "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ");";

        PGresult* res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Ошибка создания таблицы Trap: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return -1;
        }

        PQclear(res);
        return 1;
    }
};