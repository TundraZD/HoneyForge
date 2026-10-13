// src/controllers/bd_control.cpp
#include <iostream>
#include <string>
#include <limits>
#include <libpq-fe.h>

// ============================================================
// Вспомогательные утилиты для ввода
// ============================================================
static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            clearInput();
            return value;
        }
        std::cout << "Некорректный ввод. Попробуйте снова." << std::endl;
        clearInput();
    }
}

static std::string readString(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}

// ============================================================
// Меню CRUD для Profile
// ============================================================
static void menuProfile(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Profile ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            std::string login = readString("login: ");
            std::string password = readString("password_hash: ");
            std::string name = readString("name: ");
            createProfile(conn, login, password, name);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readProfileById(conn, id);
            break;
        }
        case 3:
            readAllProfiles(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            std::string login = readString("new login: ");
            std::string password = readString("new password_hash: ");
            std::string name = readString("new name: ");
            updateProfile(conn, id, login, password, name);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteProfile(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Меню CRUD для Trap
// ============================================================
static void menuTrap(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Trap ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            std::string name = readString("name: ");
            std::string type = readString("type: ");
            std::string status = readString("status: ");
            createTrap(conn, name, type, status);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readTrapById(conn, id);
            break;
        }
        case 3:
            readAllTraps(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            std::string name = readString("new name: ");
            std::string type = readString("new type: ");
            std::string status = readString("new status: ");
            updateTrap(conn, id, name, type, status);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteTrap(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Меню CRUD для Event
// ============================================================
static void menuEvent(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Event ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            int trap_id = readInt("trap_id: ");
            std::string ip = readString("source_ip: ");
            createEvent(conn, trap_id, ip);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readEventById(conn, id);
            break;
        }
        case 3:
            readAllEvents(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            int trap_id = readInt("new trap_id: ");
            std::string ip = readString("new source_ip: ");
            updateEvent(conn, id, trap_id, ip);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteEvent(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Меню CRUD для Session
// ============================================================
static void menuSession(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Session ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            int trap_id = readInt("trap_id: ");
            int event_id = readInt("event_id: ");
            std::string started = readString("started_at (YYYY-MM-DD HH:MM:SS): ");
            std::string ended = readString("ended_at (пусто = NULL): ");
            createSession(conn, trap_id, event_id, started, ended);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readSessionById(conn, id);
            break;
        }
        case 3:
            readAllSessions(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            int trap_id = readInt("new trap_id: ");
            int event_id = readInt("new event_id: ");
            std::string started = readString("new started_at: ");
            std::string ended = readString("new ended_at (пусто = NULL): ");
            updateSession(conn, id, trap_id, event_id, started, ended);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteSession(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Меню CRUD для Honeytoken
// ============================================================
static void menuHoneytoken(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Honeytoken ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            int trap_id = readInt("trap_id: ");
            std::string token = readString("token: ");
            createHoneytoken(conn, trap_id, token);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readHoneytokenById(conn, id);
            break;
        }
        case 3:
            readAllHoneytokens(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            int trap_id = readInt("new trap_id: ");
            std::string token = readString("new token: ");
            std::string triggered = readString("new triggered_at (пусто = NULL): ");
            updateHoneytoken(conn, id, trap_id, token, triggered);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteHoneytoken(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Меню CRUD для Audit
// ============================================================
static void menuAudit(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Audit ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            int profile_id = readInt("profile_id: ");
            int session_id = readInt("session_id: ");
            createAudit(conn, profile_id, session_id);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readAuditById(conn, id);
            break;
        }
        case 3:
            readAllAudits(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            int profile_id = readInt("new profile_id: ");
            int session_id = readInt("new session_id: ");
            updateAudit(conn, id, profile_id, session_id);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteAudit(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Меню CRUD для Artifact
// ============================================================
static void menuArtifact(PGconn* conn) {
    while (true) {
        std::cout << "\n=== Artifact ===\n"
            << "1. Create\n"
            << "2. Read (by id)\n"
            << "3. Read All\n"
            << "4. Update\n"
            << "5. Delete\n"
            << "0. Назад\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: {
            int event_id = readInt("event_id: ");
            std::string file_name = readString("file_name: ");
            std::string file_path = readString("file_path: ");
            std::string file_hash = readString("file_hash: ");
            createArtifact(conn, event_id, file_name, file_path, file_hash);
            break;
        }
        case 2: {
            int id = readInt("id: ");
            readArtifactById(conn, id);
            break;
        }
        case 3:
            readAllArtifacts(conn);
            break;
        case 4: {
            int id = readInt("id: ");
            int event_id = readInt("new event_id: ");
            std::string file_name = readString("new file_name: ");
            std::string file_path = readString("new file_path: ");
            std::string file_hash = readString("new file_hash: ");
            updateArtifact(conn, id, event_id, file_name, file_path, file_hash);
            break;
        }
        case 5: {
            int id = readInt("id: ");
            deleteArtifact(conn, id);
            break;
        }
        case 0: return;
        default: std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Главное меню
// ============================================================
static void mainMenu(PGconn* conn) {
    while (true) {
        std::cout << "\n=========== BD Control ===========\n"
            << "1. Profile\n"
            << "2. Trap\n"
            << "3. Event\n"
            << "4. Session\n"
            << "5. Honeytoken\n"
            << "6. Audit\n"
            << "7. Artifact\n"
            << "0. Выход\n"
            << "Выбор: ";
        int choice;
        std::cin >> choice;
        clearInput();

        switch (choice) {
        case 1: menuProfile(conn);    break;
        case 2: menuTrap(conn);       break;
        case 3: menuEvent(conn);      break;
        case 4: menuSession(conn);    break;
        case 5: menuHoneytoken(conn); break;
        case 6: menuAudit(conn);      break;
        case 7: menuArtifact(conn);   break;
        case 0:
            std::cout << "Выход." << std::endl;
            return;
        default:
            std::cout << "Неверный пункт." << std::endl;
        }
    }
}

// ============================================================
// Точка входа модуля BD Control (вызывается из main.cpp)
// ============================================================
void runBdControl(PGconn* conn) {
    if (!conn) {
        std::cerr << "runBdControl: соединение с БД не установлено." << std::endl;
        return;
    }
    mainMenu(conn);
}