#pragma once

#include <QList>
#include <QtSql/QSqlDatabase>
#include "Movie.h"

class MovieRepository
{
public:
    explicit MovieRepository(const QSqlDatabase &db);

    QList<Movie> loadAll() const;
    bool insert(Movie &movie);
    bool update(const Movie &movie);
    bool remove(int id);
    QString lastError() const;

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};
