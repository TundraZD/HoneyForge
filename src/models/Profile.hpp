// src/models/Profile.hpp
#pragma once
#include <string>
#include <libpq-fe.h> // Библиотека PostgreSQL
#include <iostream>

class Profile {
public:
    // Метод для создания таблицы
    static bool createTable(PGconn* conn) {
        if (!conn) {
            std::cerr << "Connection is null!" << std::endl;
            return false;
        }

        // SQL запрос на создание таблицы
        // SERIAL - автоинкрементный ID
        // VARCHAR - строки переменной длины
        // UNIQUE - уникальность логина
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS Profile ("
            "id SERIAL PRIMARY KEY, "
            "login VARCHAR(50) UNIQUE NOT NULL, "
            "password_hash VARCHAR(255) NOT NULL, "
            "name VARCHAR(100) NOT NULL"
            ");";

        // Выполняем запрос
        PGresult* res = PQexec(conn, sql);

        // Проверяем статус выполнения
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Ошибка создания таблицы: " << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return false;
        }

        // Очищаем память результата
        PQclear(res);
        std::cout << "Таблица Profile успешно создана (или уже существует)." << std::endl;
        return true;
    }

    // Здесь можно добавить методы добавления пользователя (INSERT) и т.д.
};