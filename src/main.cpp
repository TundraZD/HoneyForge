// src/main.cpp
#include <iostream>
#include <libpq-fe.h>
#include "models/Profile.hpp" // Подключаем наш заголовок
#include "controllers/Profile_crud.cpp"

#ifdef _WIN32
#include <windows.h>

#endif


int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
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
    //========================================================
        // ТЕСТ CRUD
        // ========================================================

        std::cout << "\n===== CREATE =====" << std::endl;
    createProfile(conn, "ivan", "hash_ivan_123", "Иван Иванов");
    createProfile(conn, "petr", "hash_petr_456", "Пётр Петров");
    createProfile(conn, "anna", "hash_anna_789", "Анна Сидорова");

    std::cout << "\n===== READ ALL =====" << std::endl;
    readAllProfiles(conn);

    std::cout << "\n===== READ BY ID =====" << std::endl;
    readProfileById(conn, 1);
    readProfileById(conn, 999); // несуществующий

    std::cout << "\n===== UPDATE =====" << std::endl;
    updateProfile(conn, 1, "ivan_new", "hash_new_000", "Иван Обновлённый");
    readProfileById(conn, 1);

    std::cout << "\n===== DELETE =====" << std::endl;
    deleteProfile(conn, 2);
    deleteProfile(conn, 999); // несуществующий

    std::cout << "\n===== READ ALL ПОСЛЕ УДАЛЕНИЯ =====" << std::endl;
    readAllProfiles(conn);

    // ========================================================

    // Закрываем соединение
    PQfinish(conn);
    return 0;
}