#pragma once

#include <QString>

struct Settings {
    int     id              = 1;
    QString theme           = "light";  // "light" | "dark"
    QString sortColumn      = "title";  // "title" | "year" | "rating" | "genre"
    QString sortOrder       = "asc";    // "asc" | "desc"
    int     moviesPerPage   = 50;
};
