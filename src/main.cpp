// src/main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <libpq-fe.h>

// Подключаем контроллеры CRUD
#include "controllers/Profile_crud.cpp"
#include "controllers/Trap_crud.cpp"
#include "controllers/Event_crud.cpp"
#include "controllers/Session_crud.cpp"
#include "controllers/Honeytoken_crud.cpp"
#include "controllers/Audit_crud.cpp"
#include "controllers/Artifact_crud.cpp"

// Подключаем модуль BD Control (меню)
#include "controllers/bd_control.cpp"

// Подключаем заголовочные файлы моделей
#include "models/Profile.hpp"
#include "models/Trap.hpp"
#include "models/Event.hpp"
#include "models/Session.hpp"
#include "models/Honeytoken.hpp"
#include "models/Audit.hpp"
#include "models/Artifact.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Строка подключения
    const char* conninfo =
        "host=localhost port=5432 dbname=postgres user=postgres password=1234";

    // Устанавливаем соединение
    PGconn* conn = PQconnectdb(conninfo);

    // Проверяем успешность подключения
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Ошибка подключения к базе данных: "
                  << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }

    std::cout << "Подключение к PostgreSQL успешно!" << std::endl;
    std::cout << "===================================" << std::endl;

    // ========================================================
    // ИНИЦИАЛИЗАЦИЯ ТАБЛИЦ
    // Порядок важен: независимые -> зависимые.
    // Каждый метод createTable сам проверяет наличие таблицы
    // и возвращает:
    //   1  — таблица создана впервые
    //   0  — таблица уже существовала
    //  -1  — ошибка
    // ========================================================

    // Описание задачи: имя таблицы + указатель на метод createTable
    struct TableTask {
        std::string name;
        int (*createFn)(PGconn*);
    };

    TableTask tasks[] = {
        { "Profile",    &Profile::createTable    },
        { "Trap",       &Trap::createTable       },
        { "Event",      &Event::createTable      },
        { "Honeytoken", &Honeytoken::createTable },
        { "Session",    &Session::createTable    },
        { "Artifact",   &Artifact::createTable   },
        { "Audit",      &Audit::createTable      },
    };

    int totalTables = sizeof(tasks) / sizeof(tasks[0]);

    // Списки для итогового отчёта
    std::vector<std::string> createdNow;   // созданные впервые
    std::vector<std::string> alreadyHave;  // уже существовавшие
    std::vector<std::string> failed;       // те, где произошла ошибка

    std::cout << "\n--- Инициализация таблиц ---" << std::endl;

    for (int i = 0; i < totalTables; ++i) {
        int result = tasks[i].createFn(conn);
        if (result == 1) {
            createdNow.push_back(tasks[i].name);
        } else if (result == 0) {
            alreadyHave.push_back(tasks[i].name);
        } else {
            failed.push_back(tasks[i].name);
        }
    }

    // ========================================================
    // ИТОГ
    // ========================================================
    std::cout << std::endl;

    // Сообщаем о реально созданных таблицах (если есть)
    if (!createdNow.empty()) {
        std::cout << "Созданы впервые: ";
        for (size_t i = 0; i < createdNow.size(); ++i) {
            std::cout << createdNow[i];
            if (i + 1 < createdNow.size()) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Если всё уже было — одна общая строка, без перечисления
    if (createdNow.empty() && failed.empty()) {
        std::cout << "Все таблицы уже существуют в БД. "
                     "Повторное создание не требуется." << std::endl;
    }

    // Ошибки — отдельным блоком
    if (!failed.empty()) {
        std::cout << "Ошибки при инициализации: ";
        for (size_t i = 0; i < failed.size(); ++i) {
            std::cout << failed[i];
            if (i + 1 < failed.size()) std::cout << ", ";
        }
        std::cout << std::endl;
    }



    std::cout << "===================================" << std::endl;

    // ========================================================
   // ЗАПУСК ИНТЕРАКТИВНОГО МЕНЮ BD CONTROL (CRUD)
   // ========================================================
    runBdControl(conn);

    // Закрываем соединение
    PQfinish(conn);
    return 0;
}