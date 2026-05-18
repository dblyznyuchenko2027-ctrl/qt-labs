#include "MovieTableModel.h"

#include <QObject>

MovieTableModel::MovieTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

int MovieTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_movies.size();
}

int MovieTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return COL_COUNT;
}

QVariant MovieTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_movies.size())
        return {};

    const Movie &m = m_movies.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case COL_ID:     return m.id;
        case COL_TITLE:  return m.title;
        case COL_GENRE:  return m.genre;
        case COL_YEAR:   return m.year;
        case COL_STATUS: return m.status;
        case COL_RATING: return m.rating == 0 ? QString("—") : QString::number(m.rating);
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == COL_ID || index.column() == COL_YEAR || index.column() == COL_RATING)
            return int(Qt::AlignCenter);
    }
    return {};
}

QVariant MovieTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case COL_ID:     return QObject::tr("ID");
        case COL_TITLE:  return QObject::tr("Назва");
        case COL_GENRE:  return QObject::tr("Жанр");
        case COL_YEAR:   return QObject::tr("Рік");
        case COL_STATUS: return QObject::tr("Статус");
        case COL_RATING: return QObject::tr("Оцінка");
        }
    }
    return {};
}

void MovieTableModel::setMovies(const QList<Movie> &movies)
{
    beginResetModel();
    m_movies = movies;
    endResetModel();
}

void MovieTableModel::addMovie(const Movie &movie)
{
    beginInsertRows({}, m_movies.size(), m_movies.size());
    m_movies.append(movie);
    endInsertRows();
}

void MovieTableModel::updateMovie(int row, const Movie &movie)
{
    if (row < 0 || row >= m_movies.size()) return;
    m_movies[row] = movie;
    emit dataChanged(index(row, 0), index(row, COL_COUNT - 1));
}

void MovieTableModel::removeMovie(int row)
{
    if (row < 0 || row >= m_movies.size()) return;
    beginRemoveRows({}, row, row);
    m_movies.removeAt(row);
    endRemoveRows();
}

Movie MovieTableModel::movieAt(int row) const
{
    if (row < 0 || row >= m_movies.size()) return {};
    return m_movies.at(row);
}
