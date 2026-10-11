cmake_minimum_required(VERSION 3.10)
project(HoneyForge)

set(CMAKE_CXX_STANDARD 17)

# --- БЛОК ПОИСКА POSTGRESQL ---
# Пытаемся найти PostgreSQL
find_package(PostgreSQL REQUIRED)

# Если CMake нашел его, эти переменные будут заполнены.
# Добавляем пути к заголовочным файлам (где лежит libpq-fe.h)
include_directories(${PostgreSQL_INCLUDE_DIRS})

add_executable(HoneyForge
    src/main.cpp
    # Если у вас есть .cpp файлы, добавьте их здесь
    # src/models/Profile.cpp 
)

# Линкуем библиотеку (чтобы линковщик нашел -lpq)
target_link_libraries(HoneyForge PRIVATE ${PostgreSQL_LIBRARIES})