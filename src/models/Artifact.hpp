// src/models/Artifact.hpp
#pragma once
#include <libpq-fe.h> // Библиотека PostgreSQL
#include <iostream>
#include <string>

class Artifact {
public:
    // Метод для создания таблицы.
    // Возвращает: 1 — создана, 0 — уже была, -1 — ошибка.
    static int createTable(PGconn* conn) {
        if (!conn) {
            std::cerr << "Connection is null!" << std::endl;
            return -1;
        }

        // Шаг 1. Проверяем наличие таблицы artifact
        const char* checkSql =
            "SELECT EXISTS ("
            "  SELECT 1 FROM information_schema.tables "
            "  WHERE table_schema = 'public' AND table_name = 'artifact'"
            ");";

        PGresult* checkRes = PQexec(conn, checkSql);
        if (PQresultStatus(checkRes) != PGRES_TUPLES_OK) {
            std::cerr << "Ошибка проверки таблицы Artifact: "
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
        // Ссылается на Event(id) с каскадным удалением.
        // file_hash — 64 символа под SHA-256.
        const char* sql =
            "CREATE TABLE IF NOT EXISTS Artifact ("
            "id SERIAL PRIMARY KEY, "
            "event_id INTEGER REFERENCES Event(id) ON DELETE CASCADE, "
            "file_name VARCHAR(255), "
            "file_path TEXT, "
            "file_hash VARCHAR(64), "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ");";

        PGresult* res = PQexec(conn, sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Ошибка создания таблицы Artifact: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return -1;
        }

        PQclear(res);
        return 1;
    }
};