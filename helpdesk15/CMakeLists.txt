cmake_minimum_required(VERSION 3.16)
project(helpdesk15 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ticketdialog.cpp
    ticketdialog.h
    ticketdialog.ui
    ticket.h
    tickettablemodel.cpp
    tickettablemodel.h
    ticketfilterproxymodel.cpp
    ticketfilterproxymodel.h
    csvticketrepository.cpp
    csvticketrepository.h
)

qt_add_executable(helpdesk15 ${SOURCES})

target_link_libraries(helpdesk15 PRIVATE Qt6::Widgets)
