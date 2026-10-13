// src/models/Profile.hpp
#pragma once
#include <string>
#include <libpq-fe.h> // Библиотека PostgreSQL
#include <iostream>

class Profile {
public:
    // Метод для создания таблицы.
    // Возвращает:
    //   1  — таблица была создана впервые
    //   0  — таблица уже существовала (создание не потребовалось)
    //  -1  — ошибка (нет соединения, сбой запроса и т.п.)
    static int createTable(PGconn* conn) {
        if (!conn) {
            std::cerr << "Connection is null!" << std::endl;
            return -1;
        }

        // Шаг 1. Проверяем, существует ли таблица в схеме public.
        // Это нужно, чтобы не выполнять CREATE TABLE повторно
        // и не выводить лишние сообщения в консоль.
        const char* checkSql =
            "SELECT EXISTS ("
            "  SELECT 1 FROM information_schema.tables "
            "  WHERE table_schema = 'public' AND table_name = 'profile'"
            ");";

        PGresult* checkRes = PQexec(conn, checkSql);

        // Проверяем статус выполнения проверочного запроса
        if (PQresultStatus(checkRes) != PGRES_TUPLES_OK) {
            std::cerr << "Ошибка проверки таблицы Profile: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(checkRes);
            return -1;
        }

        // Если таблица уже есть — возвращаем 0 и выходим,
        // ничего не создаём и не выводим сообщений.
        bool existed = (std::string(PQgetvalue(checkRes, 0, 0)) == "t");
        PQclear(checkRes);
        if (existed) {
            return 0;
        }

        // Шаг 2. Таблицы ещё нет — создаём её.
        // SERIAL   — автоинкрементный ID
        // VARCHAR  — строки переменной длины
        // UNIQUE   — уникальность логина
        // NOT NULL — обязательные поля
        const char* sql =
            "CREATE TABLE IF NOT EXISTS Profile ("
            "id SERIAL PRIMARY KEY, "
            "login VARCHAR(50) UNIQUE NOT NULL, "
            "password_hash VARCHAR(255) NOT NULL, "
            "name VARCHAR(100) NOT NULL, "
            "trap_id INTEGER"
            ");";

        // Выполняем запрос на создание таблицы
        PGresult* res = PQexec(conn, sql);

        // Проверяем статус выполнения
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Ошибка создания таблицы Profile: "
                << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return -1;
        }

        // Очищаем память результата
        PQclear(res);

        // Таблица успешно создана впервые
        return 1;
    }

    // Здесь можно добавить методы добавления пользователя (INSERT) и т.д.
};