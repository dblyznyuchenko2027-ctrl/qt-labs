#include "MovieRepository.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

MovieRepository::MovieRepository(const QSqlDatabase &db)
    : m_db(db)
{}

QList<Movie> MovieRepository::loadAll() const
{
    QList<Movie> items;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT id, title, genre, year, status, rating, comment "
                    "FROM movies ORDER BY id")) {
        m_lastError = query.lastError().text();
        qDebug() << "MovieRepository::loadAll error:" << m_lastError;
        return items;
    }

    while (query.next()) {
        Movie m;
        m.id      = query.value(0).toInt();
        m.title   = query.value(1).toString();
        m.genre   = query.value(2).toString();
        m.year    = query.value(3).toInt();
        m.status  = query.value(4).toString();
        m.rating  = query.value(5).toInt();
        m.comment = query.value(6).toString();
        items.append(m);
    }
    return items;
}

bool MovieRepository::insert(Movie &movie)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO movies (title, genre, year, status, rating, comment) "
        "VALUES (:title, :genre, :year, :status, :rating, :comment)"
    );
    query.bindValue(":title",   movie.title);
    query.bindValue(":genre",   movie.genre);
    query.bindValue(":year",    movie.year);
    query.bindValue(":status",  movie.status);
    query.bindValue(":rating",  movie.rating);
    query.bindValue(":comment", movie.comment);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "MovieRepository::insert error:" << m_lastError;
        return false;
    }
    movie.id = query.lastInsertId().toInt();
    return true;
}

bool MovieRepository::update(const Movie &movie)
{
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE movies SET "
        "  title   = :title,"
        "  genre   = :genre,"
        "  year    = :year,"
        "  status  = :status,"
        "  rating  = :rating,"
        "  comment = :comment "
        "WHERE id = :id"
    );
    query.bindValue(":title",   movie.title);
    query.bindValue(":genre",   movie.genre);
    query.bindValue(":year",    movie.year);
    query.bindValue(":status",  movie.status);
    query.bindValue(":rating",  movie.rating);
    query.bindValue(":comment", movie.comment);
    query.bindValue(":id",      movie.id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "MovieRepository::update error:" << m_lastError;
        return false;
    }
    return true;
}

bool MovieRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM movies WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "MovieRepository::remove error:" << m_lastError;
        return false;
    }
    return true;
}

QString MovieRepository::lastError() const
{
    return m_lastError;
}
