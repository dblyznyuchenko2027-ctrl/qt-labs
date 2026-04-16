cmake_minimum_required(VERSION 3.16)
project(PasswordManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets
    Network     # PR19: QNetworkAccessManager
    Concurrent  # PR20: QtConcurrent::run()
    Sql
)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    passwordleakchecker.h passwordleakchecker.cpp   # PR19
    batchcheckresult.h                               # PR20
    batchpasswordchecker.h batchpasswordchecker.cpp  # PR20
    passwordtablemodel.h passwordtablemodel.cpp
    databasemanager.h databasemanager.cpp
    resources.qrc
)

qt_add_executable(PasswordManager MANUAL_FINALIZATION ${PROJECT_SOURCES})

target_link_libraries(PasswordManager PRIVATE
    Qt6::Core Qt6::Widgets
    Qt6::Network    # PR19
    Qt6::Concurrent # PR20
    Qt6::Sql
)

set_target_properties(PasswordManager PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
qt_finalize_executable(PasswordManager)
