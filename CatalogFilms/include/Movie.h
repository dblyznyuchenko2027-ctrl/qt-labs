#pragma once

#include <QString>

struct Movie {
    int     id          = 0;
    QString title;
    QString genre;
    int     year        = 2024;
    QString status;     // "Переглянуто" | "Хочу подивитися"
    int     rating      = 0; // 0 = not rated, 1-10
    QString comment;

    bool isValid() const { return id > 0 && !title.isEmpty(); }
};
