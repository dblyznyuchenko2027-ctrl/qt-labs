cmake_minimum_required(VERSION 3.16)

project(PasswordManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# -----------------------------------------------------------------------
# Qt пакети
# Qt Network потрібен для QNetworkAccessManager / QNetworkReply
# -----------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network      # <-- ПР19: мережеві запити
    Sql          # збережено з попередніх ПР
)

# -----------------------------------------------------------------------
# Вихідні файли
# -----------------------------------------------------------------------
set(PROJECT_SOURCES
    main.cpp

    # Головне вікно
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui

    # Мережевий шар (ПР19)
    passwordleakchecker.h
    passwordleakchecker.cpp

    # Модель таблиці (з попередніх ПР)
    passwordtablemodel.h
    passwordtablemodel.cpp

    # Репозиторій / БД (з попередніх ПР)
    databasemanager.h
    databasemanager.cpp

    # Ресурси
    resources.qrc
)

qt_add_executable(PasswordManager
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# -----------------------------------------------------------------------
# Підключення бібліотек
# Qt::Network обов'язково для ПР19
# -----------------------------------------------------------------------
target_link_libraries(PasswordManager PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network     # <-- підключаємо модуль Network
    Qt6::Sql
)

# -----------------------------------------------------------------------
# Налаштування для різних платформ
# -----------------------------------------------------------------------
set_target_properties(PasswordManager PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE    TRUE
)

qt_finalize_executable(PasswordManager)
