cmake_minimum_required(VERSION 3.16)
project(CatalogFilms VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Generated headers from .ui files end up in the build dir
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MovieDialog.cpp
    src/SettingsWindow.cpp
    src/DatabaseManager.cpp
    src/MovieRepository.cpp
    src/SettingsRepository.cpp
    src/MovieService.cpp
    src/ValidationService.cpp
    src/SettingsManager.cpp
    src/SearchManager.cpp
    src/MovieTableModel.cpp
    src/MovieTableWidget.cpp
)

set(PROJECT_HEADERS
    include/MainWindow.h
    include/MovieDialog.h
    include/SettingsWindow.h
    include/Movie.h
    include/Settings.h
    include/DatabaseManager.h
    include/MovieRepository.h
    include/SettingsRepository.h
    include/MovieService.h
    include/ValidationService.h
    include/SettingsManager.h
    include/SearchManager.h
    include/MovieTableModel.h
    include/MovieTableWidget.h
)

set(PROJECT_UI
    ui/MainWindow.ui
    ui/MovieDialog.ui
    ui/SettingsWindow.ui
)

qt_add_executable(CatalogFilms
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
)

target_include_directories(CatalogFilms PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(CatalogFilms PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

set_target_properties(CatalogFilms PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
