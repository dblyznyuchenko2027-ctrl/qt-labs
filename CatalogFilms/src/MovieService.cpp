#include "MovieService.h"
#include "ValidationService.h"

MovieService::MovieService(MovieRepository *repo)
    : m_repo(repo)
{}

QList<Movie> MovieService::allMovies() const
{
    return m_repo->loadAll();
}

bool MovieService::addMovie(Movie &movie)
{
    QString err;
    if (!ValidationService::validateMovie(movie, err)) {
        m_lastError = err;
        return false;
    }
    if (!m_repo->insert(movie)) {
        m_lastError = m_repo->lastError();
        return false;
    }
    return true;
}

bool MovieService::updateMovie(const Movie &movie)
{
    QString err;
    if (!ValidationService::validateMovie(movie, err)) {
        m_lastError = err;
        return false;
    }
    if (!m_repo->update(movie)) {
        m_lastError = m_repo->lastError();
        return false;
    }
    return true;
}

bool MovieService::removeMovie(int id)
{
    if (!m_repo->remove(id)) {
        m_lastError = m_repo->lastError();
        return false;
    }
    return true;
}

QString MovieService::lastError() const
{
    return m_lastError;
}
