#pragma once

#include <QList>
#include "Movie.h"
#include "MovieRepository.h"

class MovieService
{
public:
    explicit MovieService(MovieRepository *repo);

    QList<Movie> allMovies() const;
    bool addMovie(Movie &movie);
    bool updateMovie(const Movie &movie);
    bool removeMovie(int id);
    QString lastError() const;

private:
    MovieRepository *m_repo;
    QString m_lastError;
};
