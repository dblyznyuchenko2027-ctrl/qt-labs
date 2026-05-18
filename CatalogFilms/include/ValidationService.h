#pragma once

#include <QString>
#include "Movie.h"

class ValidationService
{
public:
    static bool validateMovie(const Movie &movie, QString &errorMessage);
};
