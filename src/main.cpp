// src/main.cpp
#include <iostream>
#include <libpq-fe.h>
#include "models/Profile.hpp" // Подключаем наш заголовок

int main() {
    // Строка подключения
    // Формат: "host=... port=... dbname=... user=... password=..."
    const char* conninfo = "host=localhost port=5432 dbname=postgres user=postgres password=1234";

    // Устанавливаем соединение
    PGconn* conn = PQconnectdb(conninfo);

    // Проверяем успешность подключения
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Ошибка подключения к базе данных: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }

    std::cout << "Подключение к PostgreSQL успешно!" << std::endl;

    // Вызываем метод создания таблицы из нашего класса Profile
    Profile::createTable(conn);

    // Закрываем соединение
    PQfinish(conn);
    return 0;
}